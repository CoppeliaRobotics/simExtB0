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
#include "simPlusPlus/Plugin.h"
#include "simPlusPlus/Handle.h"
#include "plugin.h"
#include "stubs.h"
#include "config.h"
#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>
#include <b0/service_client.h>
#include <b0/service_server.h>

using sim::Handle;

using Node = b0::Node;
using Socket = b0::Socket;
using Publisher = b0::Publisher;
using Subscriber = b0::Subscriber;
using ServiceClient = b0::ServiceClient;
using ServiceServer = b0::ServiceServer;

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

class Plugin : public sim::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        setExtVersion("BlueZero Interface Plugin");
        setBuildDate(BUILD_DATE);

        if (!b0::isInitialized())
            b0::init();
    }

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

    void nodeCreate(nodeCreate_in *in, nodeCreate_out *out)
    {
        auto *pnode = new Node(in->name);

        auto *meta = new Metadata;
        meta->handle = Handle<Node>::str(pnode);
        pnode->setUserData(meta);

        out->handle = meta->handle;
        handles.insert(meta->handle);
    }

    void nodeSetAnnounceTimeout(nodeSetAnnounceTimeout_in *in, nodeSetAnnounceTimeout_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->handle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");
        pnode->setAnnounceTimeout(in->timeout);
    }

    void nodeInit(nodeInit_in *in, nodeInit_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->handle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");
        pnode->init();
        out->name = pnode->getName();
    }

    void nodeSpinOnce(nodeSpinOnce_in *in, nodeSpinOnce_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->handle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");
        pnode->spinOnce();
    }

    void nodeCleanup(nodeCleanup_in *in, nodeCleanup_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->handle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");
        pnode->cleanup();
    }

    void nodeDestroy(nodeDestroy_in *in, nodeDestroy_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->handle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");

        auto *meta = Metadata::get(pnode);
        handles.erase(meta->handle);
        delete meta;

        delete pnode;
    }

    void socketInit(socketInit_in *in, socketInit_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        psock->init();
    }

    void socketSpinOnce(socketSpinOnce_in *in, socketSpinOnce_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        psock->spinOnce();
    }

    void socketPoll(socketPoll_in *in, socketPoll_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        out->result = psock->poll();
    }

    void socketRead(socketRead_in *in, socketRead_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        psock->readRaw(out->payload);
    }

    void socketWrite(socketWrite_in *in, socketWrite_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        psock->writeRaw(in->payload);
    }

    void socketCleanup(socketCleanup_in *in, socketCleanup_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        psock->cleanup();
    }

    void publisherCreate(publisherCreate_in *in, publisherCreate_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->nodeHandle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");
        auto *ppub = new Publisher(pnode, in->topic, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = Handle<Publisher>::str(ppub);
        ppub->setUserData(meta);

        out->handle = meta->handle;
        handles.insert(meta->handle);
    }

    void publisherPublish(publisherPublish_in *in, publisherPublish_out *out)
    {
        auto *ppub = Handle<Publisher>::obj(in->handle);
        if(!ppub)
            throw std::runtime_error("Invalid publisher handle");
        ppub->publish(in->payload);
    }

    void publisherDestroy(publisherDestroy_in *in, publisherDestroy_out *out)
    {
        auto *ppub = Handle<Publisher>::obj(in->handle);
        if(!ppub)
            throw std::runtime_error("Invalid publisher handle");

        auto *meta = Metadata::get(ppub);
        handles.erase(meta->handle);
        delete meta;

        delete ppub;
    }

    void subscriberCreate(subscriberCreate_in *in, subscriberCreate_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->nodeHandle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");

        Subscriber::CallbackRaw callback = boost::bind(&Plugin::topicCallbackWrapper, this, in->_scriptID, in->callback, _1);
        auto *psub = new Subscriber(pnode, in->topic, callback, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = Handle<Subscriber>::str(psub);
        psub->setUserData(meta);

        out->handle = meta->handle;
        handles.insert(meta->handle);
    }

    void subscriberDestroy(subscriberDestroy_in *in, subscriberDestroy_out *out)
    {
        auto *psub = Handle<Subscriber>::obj(in->handle);
        if(!psub)
            throw std::runtime_error("Invalid subscriber handle");

        auto *meta = Metadata::get(psub);
        handles.erase(meta->handle);
        delete meta;

        delete psub;
    }

    void serviceClientCreate(serviceClientCreate_in *in, serviceClientCreate_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->nodeHandle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");
        auto *pcli = new ServiceClient(pnode, in->service, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = Handle<ServiceClient>::str(pcli);
        pcli->setUserData(meta);

        out->handle = meta->handle;
        handles.insert(meta->handle);
    }

    void serviceClientCall(serviceClientCall_in *in, serviceClientCall_out *out)
    {
        auto *pcli = Handle<ServiceClient>::obj(in->handle);
        if(!pcli)
            throw std::runtime_error("Invalid service client handle");
        pcli->call(in->payload, out->payload);
    }

    void serviceClientDestroy(serviceClientDestroy_in *in, serviceClientDestroy_out *out)
    {
        auto *pcli = Handle<ServiceClient>::obj(in->handle);
        if(!pcli)
            throw std::runtime_error("Invalid service client handle");

        auto *meta = Metadata::get(pcli);
        handles.erase(meta->handle);
        delete meta;

        delete pcli;
    }

    void serviceServerCreate(serviceServerCreate_in *in, serviceServerCreate_out *out)
    {
        auto *pnode = Handle<Node>::obj(in->nodeHandle);
        if(!pnode)
            throw std::runtime_error("Invalid node handle");

        ServiceServer::CallbackRaw callback = boost::bind(&Plugin::serviceCallbackWrapper, this, in->_scriptID, in->callback, _1, _2);
        auto *psrv = new ServiceServer(pnode, in->service, callback, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = Handle<ServiceServer>::str(psrv);
        psrv->setUserData(meta);

        out->handle = meta->handle;
        handles.insert(meta->handle);
    }

    void serviceServerDestroy(serviceServerDestroy_in *in, serviceServerDestroy_out *out)
    {
        auto *psrv = Handle<ServiceServer>::obj(in->handle);
        if(!psrv)
            throw std::runtime_error("Invalid service server handle");

        auto *meta = Metadata::get(psrv);
        handles.erase(meta->handle);
        delete meta;

        delete psrv;
    }

    void socketSetCompression(socketSetCompression_in *in, socketSetCompression_out *out)
    {
        auto *psock = Handle<Socket>::obj(in->handle);
        if(!psock)
            throw std::runtime_error("Invalid socket handle");
        psock->setCompression(in->compressionAlgorithm, in->compressionLevel);
    }

    void socketSetOption(socketSetOption_in *in, socketSetOption_out *out)
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
        else throw sim::exception("invalid option: '%s'", in->option);
    }

    void getHandles(getHandles_in *in, getHandles_out *out)
    {
        for(const std::string &handle : handles)
            out->handles.push_back(handle);
    }

private:
    std::set<std::string> handles;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
