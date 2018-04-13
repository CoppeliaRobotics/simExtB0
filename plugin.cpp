#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "v_repPlusPlus/Plugin.h"
#include "plugin.h"
#include "stubs.h"
#include "config.h"
#include <b0/b0.h>

std::set<std::string> handles;

using Node = b0::Node;
using Socket = b0::Socket;
using Publisher = b0::Publisher;
using Subscriber = b0::Subscriber;
using ServiceClient = b0::ServiceClient;
using ServiceServer = b0::ServiceServer;

// handle: a tool for pointer <--> string conversion
template<typename T>
struct Handle
{
    static std::string str(const T *t)
    {
        static boost::format fmt("%s:%lld:%d");
        return (fmt % tag() % reinterpret_cast<long long int>(t) % crc_ptr(t)).str();
    }

    static T * obj(std::string h)
    {
        boost::cmatch m;
        static boost::regex re("([^:]+):([^:]+):([^:]+)");
        if(boost::regex_match(h.c_str(), m, re) && m[1] == tag())
        {
            T *t = reinterpret_cast<T*>(boost::lexical_cast<long long int>(m[2]));
            int crc = boost::lexical_cast<int>(m[3]);
            if(crc == crc_ptr(t)) return t;
        }
        return nullptr;
    }

private:
    static std::string tag()
    {
        return "ptr";
    }

    static int crc_ptr(const T *t)
    {
        auto x = reinterpret_cast<long long int>(t);
        x = x ^ (x >> 32);
        x = x ^ (x >> 16);
        x = x ^ (x >> 8);
        x = x ^ (x >> 4);
        x = x & 0x000000000000000F;
        x = x ^ 0x0000000000000008;
        return int(x);
    }
};

template<> std::string Handle<Node>::tag() { return "b0.node"; }
template<> std::string Handle<Publisher>::tag() { return "b0.pub"; }
template<> std::string Handle<Subscriber>::tag() { return "b0.sub"; }
template<> std::string Handle<ServiceClient>::tag() { return "b0.cli"; }
template<> std::string Handle<ServiceServer>::tag() { return "b0.srv"; }

template<> Socket * Handle<Socket>::obj(std::string h)
{
    auto *ppub = Handle<Publisher>::obj(h);
    if(ppub) return ppub;
    auto *psub = Handle<Subscriber>::obj(h);
    if(psub) return psub;
    auto *pcli = Handle<ServiceClient>::obj(h);
    if(pcli) return pcli;
    auto *psrv = Handle<ServiceServer>::obj(h);
    if(psrv) return psrv;
    return nullptr;
}

struct Metadata
{
    std::string handle;

    static Metadata * get(b0::UserData *x)
    {
        return reinterpret_cast<Metadata*>(x->getUserData());
    }
};

void topicCallbackWrapper(int scriptID, std::string callback, const std::string &payload)
{
    topicCallback_in in;
    in.payload = payload;
    topicCallback_out out;
    topicCallback(scriptID, callback.c_str(), &in, &out);
}

void serviceCallbackWrapper(int scriptID, std::string callback, const std::string &req_payload, std::string &rep_payload)
{
    serviceCallback_in in;
    in.payload = req_payload;
    serviceCallback_out out;
    serviceCallback(scriptID, callback.c_str(), &in, &out);
    rep_payload = out.payload;
}

void create(SScriptCallBack *p, const char *cmd, create_in *in, create_out *out)
{
    auto *pnode = new Node(in->name);

    auto *meta = new Metadata;
    meta->handle = Handle<Node>::str(pnode);
    pnode->setUserData(meta);

    out->handle = meta->handle;
    handles.insert(meta->handle);
}

void setAnnounceTimeout(SScriptCallBack *p, const char *cmd, setAnnounceTimeout_in *in, setAnnounceTimeout_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    pnode->setAnnounceTimeout(in->timeout);
}

void init(SScriptCallBack *p, const char *cmd, init_in *in, init_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    pnode->init();
    out->name = pnode->getName();
}

void spinOnce(SScriptCallBack *p, const char *cmd, spinOnce_in *in, spinOnce_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    pnode->spinOnce();
}

void cleanup(SScriptCallBack *p, const char *cmd, cleanup_in *in, cleanup_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    pnode->cleanup();
}

void destroy(SScriptCallBack *p, const char *cmd, destroy_in *in, destroy_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");

    auto *meta = Metadata::get(pnode);
    handles.erase(meta->handle);
    delete meta;

    delete pnode;
}

void initSocket(SScriptCallBack *p, const char *cmd, initSocket_in *in, initSocket_out *out)
{
    auto *psock = Handle<Socket>::obj(in->handle);
    if(!psock)
        throw std::runtime_error("Invalid socket handle");
    psock->init();
}

void spinOnceSocket(SScriptCallBack *p, const char *cmd, spinOnceSocket_in *in, spinOnceSocket_out *out)
{
    auto *psock = Handle<Socket>::obj(in->handle);
    if(!psock)
        throw std::runtime_error("Invalid socket handle");
    psock->spinOnce();
}

void cleanupSocket(SScriptCallBack *p, const char *cmd, cleanupSocket_in *in, cleanupSocket_out *out)
{
    auto *psock = Handle<Socket>::obj(in->handle);
    if(!psock)
        throw std::runtime_error("Invalid socket handle");
    psock->cleanup();
}

void createPublisher(SScriptCallBack *p, const char *cmd, createPublisher_in *in, createPublisher_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    auto *ppub = new Publisher(pnode, in->topic, in->managed, in->notify_graph);

    auto *meta = new Metadata;
    meta->handle = Handle<Publisher>::str(ppub);
    ppub->setUserData(meta);

    out->handle = meta->handle;
    handles.insert(meta->handle);
}

void publish(SScriptCallBack *p, const char *cmd, publish_in *in, publish_out *out)
{
    auto *ppub = Handle<Publisher>::obj(in->handle);
    if(!ppub)
        throw std::runtime_error("Invalid publisher handle");
    ppub->publish(in->payload);
}

void destroyPublisher(SScriptCallBack *p, const char *cmd, destroyPublisher_in *in, destroyPublisher_out *out)
{
    auto *ppub = Handle<Publisher>::obj(in->handle);
    if(!ppub)
        throw std::runtime_error("Invalid publisher handle");

    auto *meta = Metadata::get(ppub);
    handles.erase(meta->handle);
    delete meta;

    delete ppub;
}

void createSubscriber(SScriptCallBack *p, const char *cmd, createSubscriber_in *in, createSubscriber_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    auto callback = boost::bind(topicCallbackWrapper, p->scriptID, in->callback, _1);
    auto *psub = new Subscriber(pnode, in->topic, callback, in->managed, in->notify_graph);

    auto *meta = new Metadata;
    meta->handle = Handle<Subscriber>::str(psub);
    psub->setUserData(meta);

    out->handle = meta->handle;
    handles.insert(meta->handle);
}

void destroySubscriber(SScriptCallBack *p, const char *cmd, destroySubscriber_in *in, destroySubscriber_out *out)
{
    auto *psub = Handle<Subscriber>::obj(in->handle);
    if(!psub)
        throw std::runtime_error("Invalid subscriber handle");

    auto *meta = Metadata::get(psub);
    handles.erase(meta->handle);
    delete meta;

    delete psub;
}

void createServiceClient(SScriptCallBack *p, const char *cmd, createServiceClient_in *in, createServiceClient_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    auto *pcli = new ServiceClient(pnode, in->service, in->managed, in->notify_graph);

    auto *meta = new Metadata;
    meta->handle = Handle<ServiceClient>::str(pcli);
    pcli->setUserData(meta);

    out->handle = meta->handle;
    handles.insert(meta->handle);
}

void call(SScriptCallBack *p, const char *cmd, call_in *in, call_out *out)
{
    auto *pcli = Handle<ServiceClient>::obj(in->handle);
    if(!pcli)
        throw std::runtime_error("Invalid service client handle");
    pcli->call(in->payload, out->payload);
}

void destroyServiceClient(SScriptCallBack *p, const char *cmd, destroyServiceClient_in *in, destroyServiceClient_out *out)
{
    auto *pcli = Handle<ServiceClient>::obj(in->handle);
    if(!pcli)
        throw std::runtime_error("Invalid service client handle");

    auto *meta = Metadata::get(pcli);
    handles.erase(meta->handle);
    delete meta;

    delete pcli;
}

void createServiceServer(SScriptCallBack *p, const char *cmd, createServiceServer_in *in, createServiceServer_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    if(!pnode)
        throw std::runtime_error("Invalid node handle");
    auto callback = boost::bind(serviceCallbackWrapper, p->scriptID, in->callback, _1, _2);
    auto *psrv = new ServiceServer(pnode, in->service, callback, in->managed, in->notify_graph);

    auto *meta = new Metadata;
    meta->handle = Handle<ServiceServer>::str(psrv);
    psrv->setUserData(meta);

    out->handle = meta->handle;
    handles.insert(meta->handle);
}

void destroyServiceServer(SScriptCallBack *p, const char *cmd, destroyServiceServer_in *in, destroyServiceServer_out *out)
{
    auto *psrv = Handle<ServiceServer>::obj(in->handle);
    if(!psrv)
        throw std::runtime_error("Invalid service server handle");

    auto *meta = Metadata::get(psrv);
    handles.erase(meta->handle);
    delete meta;

    delete psrv;
}

void setCompression(SScriptCallBack *p, const char *cmd, setCompression_in *in, setCompression_out *out)
{
    auto *psock = Handle<Socket>::obj(in->handle);
    if(!psock)
        throw std::runtime_error("Invalid socket handle");
    psock->setCompression(in->compression_algorithm, in->compression_level);
}

void setSocketOption(SScriptCallBack *p, const char *cmd, setSocketOption_in *in, setSocketOption_out *out)
{
    auto *psock = Handle<Socket>::obj(in->handle);
    if(!psock)
        throw std::runtime_error("Invalid socket handle");

    int v = in->value;
    if(in->option == "lingerPeriod") psock->setLingerPeriod(v);
    else if(in->option == "backlog") psock->setBacklog(v);
    else if(in->option == "readTimeout") psock->setReadTimeout(v);
    else if(in->option == "writeTimeout") psock->setWriteTimeout(v);
    else if(in->option == "immediate") psock->setImmediate(v);
    else if(in->option == "conflate") psock->setConflate(v);
    else if(in->option == "readHWM") psock->setReadHWM(v);
    else if(in->option == "writeHWM") psock->setWriteHWM(v);
    else throw (boost::format("invalid option: '%s'") % in->option).str();
}

void getHandles(SScriptCallBack *p, const char *cmd, getHandles_in *in, getHandles_out *out)
{
    for(const std::string &handle : handles)
        out->handles.push_back(handle);
}

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");
    }
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
