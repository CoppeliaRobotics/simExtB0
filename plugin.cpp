#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <boost/bind.hpp>
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
using sim::Handles;

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

        if(!b0::isInitialized())
            b0::init();
    }

    void onScriptStateDestroyed(int scriptID)
    {
        for(auto obj : nodeHandles.find(scriptID))
            delete nodeHandles.remove(obj);
        for(auto obj : publisherHandles.find(scriptID))
            delete publisherHandles.remove(obj);
        for(auto obj : subscriberHandles.find(scriptID))
            delete subscriberHandles.remove(obj);
        for(auto obj : serviceClientHandles.find(scriptID))
            delete serviceClientHandles.remove(obj);
        for(auto obj : serviceServerHandles.find(scriptID))
            delete serviceServerHandles.remove(obj);
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
        meta->handle = nodeHandles.add(pnode, in->_.scriptID);
        pnode->setUserData(meta);

        out->handle = meta->handle;
    }

    void nodeSetAnnounceTimeout(nodeSetAnnounceTimeout_in *in, nodeSetAnnounceTimeout_out *out)
    {
        auto *pnode = nodeHandles.get(in->handle);
        pnode->setAnnounceTimeout(in->timeout);
    }

    void nodeInit(nodeInit_in *in, nodeInit_out *out)
    {
        auto *pnode = nodeHandles.get(in->handle);
        pnode->init();
        out->name = pnode->getName();
    }

    void nodeSpinOnce(nodeSpinOnce_in *in, nodeSpinOnce_out *out)
    {
        auto *pnode = nodeHandles.get(in->handle);
        pnode->spinOnce();
    }

    void nodeCleanup(nodeCleanup_in *in, nodeCleanup_out *out)
    {
        auto *pnode = nodeHandles.get(in->handle);
        pnode->cleanup();
    }

    void nodeDestroy(nodeDestroy_in *in, nodeDestroy_out *out)
    {
        auto *pnode = nodeHandles.get(in->handle);

        auto *meta = Metadata::get(pnode);
        delete meta;

        delete nodeHandles.remove(pnode);
    }

    void socketInit(socketInit_in *in, socketInit_out *out)
    {
        auto *psock = getSocket(in->handle);
        psock->init();
    }

    void socketSpinOnce(socketSpinOnce_in *in, socketSpinOnce_out *out)
    {
        auto *psock = getSocket(in->handle);
        psock->spinOnce();
    }

    void socketPoll(socketPoll_in *in, socketPoll_out *out)
    {
        auto *psock = getSocket(in->handle);
        out->result = psock->poll();
    }

    void socketRead(socketRead_in *in, socketRead_out *out)
    {
        auto *psock = getSocket(in->handle);
        psock->readRaw(out->payload);
    }

    void socketWrite(socketWrite_in *in, socketWrite_out *out)
    {
        auto *psock = getSocket(in->handle);
        psock->writeRaw(in->payload);
    }

    void socketCleanup(socketCleanup_in *in, socketCleanup_out *out)
    {
        auto *psock = getSocket(in->handle);
        psock->cleanup();
    }

    void publisherCreate(publisherCreate_in *in, publisherCreate_out *out)
    {
        auto *pnode = nodeHandles.get(in->nodeHandle);
        auto *ppub = new Publisher(pnode, in->topic, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = publisherHandles.add(ppub, in->_.scriptID);
        ppub->setUserData(meta);

        out->handle = meta->handle;
    }

    void publisherPublish(publisherPublish_in *in, publisherPublish_out *out)
    {
        auto *ppub = publisherHandles.get(in->handle);
        ppub->publish(in->payload);
    }

    void publisherDestroy(publisherDestroy_in *in, publisherDestroy_out *out)
    {
        auto *ppub = publisherHandles.get(in->handle);

        auto *meta = Metadata::get(ppub);
        delete meta;

        delete publisherHandles.remove(ppub);
    }

    void subscriberCreate(subscriberCreate_in *in, subscriberCreate_out *out)
    {
        auto *pnode = nodeHandles.get(in->nodeHandle);

        Subscriber::CallbackRaw callback = boost::bind(&Plugin::topicCallbackWrapper, this, in->_.scriptID, in->callback, _1);
        auto *psub = new Subscriber(pnode, in->topic, callback, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = subscriberHandles.add(psub, in->_.scriptID);
        psub->setUserData(meta);

        out->handle = meta->handle;
    }

    void subscriberDestroy(subscriberDestroy_in *in, subscriberDestroy_out *out)
    {
        auto *psub = subscriberHandles.get(in->handle);

        auto *meta = Metadata::get(psub);
        delete meta;

        delete subscriberHandles.remove(psub);
    }

    void serviceClientCreate(serviceClientCreate_in *in, serviceClientCreate_out *out)
    {
        auto *pnode = nodeHandles.get(in->nodeHandle);
        auto *pcli = new ServiceClient(pnode, in->service, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = serviceClientHandles.add(pcli, in->_.scriptID);
        pcli->setUserData(meta);

        out->handle = meta->handle;
    }

    void serviceClientCall(serviceClientCall_in *in, serviceClientCall_out *out)
    {
        auto *pcli = serviceClientHandles.get(in->handle);
        pcli->call(in->payload, out->payload);
    }

    void serviceClientDestroy(serviceClientDestroy_in *in, serviceClientDestroy_out *out)
    {
        auto *pcli = serviceClientHandles.get(in->handle);

        auto *meta = Metadata::get(pcli);
        delete meta;

        delete serviceClientHandles.remove(pcli);
    }

    void serviceServerCreate(serviceServerCreate_in *in, serviceServerCreate_out *out)
    {
        auto *pnode = nodeHandles.get(in->nodeHandle);

        ServiceServer::CallbackRaw callback = boost::bind(&Plugin::serviceCallbackWrapper, this, in->_.scriptID, in->callback, _1, _2);
        auto *psrv = new ServiceServer(pnode, in->service, callback, in->managed, in->notifyGraph);

        auto *meta = new Metadata;
        meta->handle = serviceServerHandles.add(psrv, in->_.scriptID);
        psrv->setUserData(meta);

        out->handle = meta->handle;
    }

    void serviceServerDestroy(serviceServerDestroy_in *in, serviceServerDestroy_out *out)
    {
        auto *psrv = serviceServerHandles.get(in->handle);

        auto *meta = Metadata::get(psrv);
        delete meta;

        delete serviceServerHandles.remove(psrv);
    }

    void socketSetCompression(socketSetCompression_in *in, socketSetCompression_out *out)
    {
        auto *psock = getSocket(in->handle);
        psock->setCompression(in->compressionAlgorithm, in->compressionLevel);
    }

    void socketSetOption(socketSetOption_in *in, socketSetOption_out *out)
    {
        auto *psock = getSocket(in->handle);

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
        for(const auto &x : nodeHandles.find(in->_.scriptID))
            out->handles.push_back(Handle<Node>::str(x));
        for(const auto &x : publisherHandles.find(in->_.scriptID))
            out->handles.push_back(Handle<Publisher>::str(x));
        for(const auto &x : subscriberHandles.find(in->_.scriptID))
            out->handles.push_back(Handle<Subscriber>::str(x));
        for(const auto &x : serviceClientHandles.find(in->_.scriptID))
            out->handles.push_back(Handle<ServiceClient>::str(x));
        for(const auto &x : serviceServerHandles.find(in->_.scriptID))
            out->handles.push_back(Handle<ServiceServer>::str(x));
    }

    Socket * getSocket(std::string h)
    {
        auto *ppub = Handle<Publisher>::obj(h);
        if(ppub) return publisherHandles.get(h);
        auto *psub = Handle<Subscriber>::obj(h);
        if(psub) return subscriberHandles.get(h);
        auto *pcli = Handle<ServiceClient>::obj(h);
        if(pcli) return serviceClientHandles.get(h);
        auto *psrv = Handle<ServiceServer>::obj(h);
        if(psrv) return serviceServerHandles.get(h);
        throw sim::exception("invalid handle: '%s'", h);
    }

private:
    Handles<Node> nodeHandles;
    Handles<Publisher> publisherHandles;
    Handles<Subscriber> subscriberHandles;
    Handles<ServiceClient> serviceClientHandles;
    Handles<ServiceServer> serviceServerHandles;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
