#ifndef _BAIDU_SHUTTLE_MASTER_IMPL_H_
#define _BAIDU_SHUTTLE_MASTER_IMPL_H_
#include <string>
#include <map>
#include <set>

#include "galaxy.h"
#include "ins_sdk.h"
#include "mutex.h"
#include "thread_pool.h"
#include "proto/master.pb.h"
#include "proto/serialize.pb.h"
#include "job_tracker.h"

namespace baidu {
namespace shuttle {

class MasterImpl : public Master {
public:

    MasterImpl();
    virtual ~MasterImpl();

    void Init();

    void SubmitJob(::google::protobuf::RpcController* controller,
                   const ::baidu::shuttle::SubmitJobRequest* request,
                   ::baidu::shuttle::SubmitJobResponse* response,
                   ::google::protobuf::Closure* done);
    void UpdateJob(::google::protobuf::RpcController* controller,
                   const ::baidu::shuttle::UpdateJobRequest* request,
                   ::baidu::shuttle::UpdateJobResponse* response,
                   ::google::protobuf::Closure* done);
    void KillJob(::google::protobuf::RpcController* controller,
                 const ::baidu::shuttle::KillJobRequest* request,
                 ::baidu::shuttle::KillJobResponse* response,
                 ::google::protobuf::Closure* done);
    void ListJobs(::google::protobuf::RpcController* controller,
                  const ::baidu::shuttle::ListJobsRequest* request,
                  ::baidu::shuttle::ListJobsResponse* response,
                  ::google::protobuf::Closure* done);
    void ShowJob(::google::protobuf::RpcController* controller,
                 const ::baidu::shuttle::ShowJobRequest* request,
                 ::baidu::shuttle::ShowJobResponse* response,
                 ::google::protobuf::Closure* done);
    void AssignTask(::google::protobuf::RpcController* controller,
                    const ::baidu::shuttle::AssignTaskRequest* request,
                    ::baidu::shuttle::AssignTaskResponse* response,
                    ::google::protobuf::Closure* done);
    void FinishTask(::google::protobuf::RpcController* controller,
                    const ::baidu::shuttle::FinishTaskRequest* request,
                    ::baidu::shuttle::FinishTaskResponse* response,
                    ::google::protobuf::Closure* done);

private:
    void AcquireMasterLock();
    static void OnMasterSessionTimeout(void* ctx);
    void OnSessionTimeout();
    static void OnMasterLockChange(const ::galaxy::ins::sdk::WatchParam& param,
                                   ::galaxy::ins::sdk::SDKError err);
    void OnLockChange(const std::string& lock_session_id);
    std::string SelfEndpoint();
    void RetractJob(const std::string& jobid);
    void KeepGarbageCollecting();
    void KeepDataPersistence();
    void Reload();
    Status Load(const std::string& compressed, JobCollection& jc);
    std::string Dump(const JobCollection& jc);
    bool GetJobFromNexus(JobCollection& jc);
    bool SaveJobToNexus(JobTracker* jobtracker);
    bool RemoveJobFromNexus(const std::string& jobid);

private:
    Mutex tracker_mu_;
    std::map<std::string, JobTracker*> job_trackers_;
    Mutex dead_mu_;
    std::map<std::string, JobTracker*> dead_trackers_;
    ThreadPool gc_;
    // For persistent of meta data and addressing of minion
    ::galaxy::ins::sdk::InsSDK* nexus_;
    std::set<std::string> saved_dead_jobs_;
};

}
}

#endif

