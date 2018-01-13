#include <string>
#include <vector>
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

using Node = b0::Node;
using Publisher = b0::Publisher<std::string>;
using Subscriber = b0::Subscriber<std::string>;
using ServiceClient = b0::ServiceClient<std::string, std::string>;
using ServiceServer = b0::ServiceServer<std::string, std::string>;

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
}

void setAnnounceTimeout(SScriptCallBack *p, const char *cmd, setAnnounceTimeout_in *in, setAnnounceTimeout_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    pnode->setAnnounceTimeout(in->timeout);
}

void init(SScriptCallBack *p, const char *cmd, init_in *in, init_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    pnode->init();
    out->name = pnode->getName();
}

void spin(SScriptCallBack *p, const char *cmd, spin_in *in, spin_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    pnode->spin();
}

void spinOnce(SScriptCallBack *p, const char *cmd, spinOnce_in *in, spinOnce_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    pnode->spinOnce();
}

void cleanup(SScriptCallBack *p, const char *cmd, cleanup_in *in, cleanup_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    pnode->cleanup();
}

void destroy(SScriptCallBack *p, const char *cmd, destroy_in *in, destroy_out *out)
{
    auto *pnode = Handle<Node>::obj(in->handle);
    delete Metadata::get(pnode);
    delete pnode;
}

void createPublisher(SScriptCallBack *p, const char *cmd, createPublisher_in *in, createPublisher_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    auto *ppub = new Publisher(pnode, in->topic);

    auto *meta = new Metadata;
    meta->handle = Handle<Publisher>::str(ppub);
    ppub->setUserData(meta);

    out->handle = meta->handle;
}

void publish(SScriptCallBack *p, const char *cmd, publish_in *in, publish_out *out)
{
    auto *ppub = Handle<Publisher>::obj(in->handle);
    ppub->publish(in->payload);
}

void destroyPublisher(SScriptCallBack *p, const char *cmd, destroyPublisher_in *in, destroyPublisher_out *out)
{
    auto *ppub = Handle<Publisher>::obj(in->handle);
    delete Metadata::get(ppub);
    delete ppub;
}

void createSubscriber(SScriptCallBack *p, const char *cmd, createSubscriber_in *in, createSubscriber_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    auto callback = boost::bind(topicCallbackWrapper, p->scriptID, in->callback, _1);
    auto *psub = new Subscriber(pnode, in->topic, callback);

    auto *meta = new Metadata;
    meta->handle = Handle<Subscriber>::str(psub);
    psub->setUserData(meta);

    out->handle = meta->handle;
}

void destroySubscriber(SScriptCallBack *p, const char *cmd, destroySubscriber_in *in, destroySubscriber_out *out)
{
    auto *psub = Handle<Subscriber>::obj(in->handle);
    delete Metadata::get(psub);
    delete psub;
}

void createServiceClient(SScriptCallBack *p, const char *cmd, createServiceClient_in *in, createServiceClient_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    auto *pcli = new ServiceClient(pnode, in->service);

    auto *meta = new Metadata;
    meta->handle = Handle<ServiceClient>::str(pcli);
    pcli->setUserData(meta);

    out->handle = meta->handle;
}

void call(SScriptCallBack *p, const char *cmd, call_in *in, call_out *out)
{
    auto *pcli = Handle<ServiceClient>::obj(in->handle);
    pcli->call(in->payload, out->payload);
}

void destroyServiceClient(SScriptCallBack *p, const char *cmd, destroyServiceClient_in *in, destroyServiceClient_out *out)
{
    auto *pcli = Handle<ServiceClient>::obj(in->handle);
    delete Metadata::get(pcli);
    delete pcli;
}

void createServiceServer(SScriptCallBack *p, const char *cmd, createServiceServer_in *in, createServiceServer_out *out)
{
    auto *pnode = Handle<Node>::obj(in->nodeHandle);
    auto callback = boost::bind(serviceCallbackWrapper, p->scriptID, in->callback, _1, _2);
    auto *psrv = new ServiceServer(pnode, in->service, callback);

    auto *meta = new Metadata;
    meta->handle = Handle<ServiceServer>::str(psrv);
    psrv->setUserData(meta);

    out->handle = meta->handle;
}

void destroyServiceServer(SScriptCallBack *p, const char *cmd, destroyServiceServer_in *in, destroyServiceServer_out *out)
{
    auto *psrv = Handle<ServiceServer>::obj(in->handle);
    delete Metadata::get(psrv);
    delete psrv;
}

void setCompression(SScriptCallBack *p, const char *cmd, setCompression_in *in, setCompression_out *out)
{
    auto *ppub = Handle<Publisher>::obj(in->handle);
    if(ppub)
    {
        ppub->setCompression(in->compression_algorithm, in->compression_level);
        return;
    }
    auto *pcli = Handle<ServiceClient>::obj(in->handle);
    if(pcli)
    {
        pcli->setCompression(in->compression_algorithm, in->compression_level);
        return;
    }
    auto *psrv = Handle<ServiceServer>::obj(in->handle);
    if(psrv)
    {
        psrv->setCompression(in->compression_algorithm, in->compression_level);
        return;
    }
}

void setSocketOption(SScriptCallBack *p, const char *cmd, setSocketOption_in *in, setSocketOption_out *out)
{
    b0::socket::Socket *socket = nullptr;
    if(auto *ppub = Handle<Publisher>::obj(in->handle)) socket = ppub;
    else if(auto *psub = Handle<Subscriber>::obj(in->handle)) socket = psub;
    else if(auto *pcli = Handle<ServiceClient>::obj(in->handle)) socket = pcli;
    else if(auto *psrv = Handle<ServiceServer>::obj(in->handle)) socket = psrv;
    else throw std::string("invalid socket type");

    int v = in->value;
    if(in->option == "lingerPeriod") socket->setLingerPeriod(v);
    else if(in->option == "backlog") socket->setBacklog(v);
    else if(in->option == "readTimeout") socket->setReadTimeout(v);
    else if(in->option == "writeTimeout") socket->setWriteTimeout(v);
    else if(in->option == "immediate") socket->setImmediate(v);
    else if(in->option == "conflate") socket->setConflate(v);
    else if(in->option == "readHWM") socket->setReadHWM(v);
    else if(in->option == "writeHWM") socket->setWriteHWM(v);
    else throw (boost::format("invalid option: '%s'") % in->option).str();
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
