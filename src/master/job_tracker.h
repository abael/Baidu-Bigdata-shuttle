#ifndef _BAIDU_SHUTTLE_JOB_TRACKER_H_
#define _BAIDU_SHUTTLE_JOB_TRACKER_H_
#include <string>
#include <list>
#include <queue>
#include <utility>
#include <ctime>

#include "galaxy.h"
#include "mutex.h"
#include "thread_pool.h"
#include "proto/shuttle.pb.h"
#include "proto/app_master.pb.h"
#include "resource_manager.h"
#include "common/rpc_client.h"
#include "common/filesystem.h"
#include "logging.h"

namespace baidu {
namespace shuttle {

class MasterImpl;

struct AllocateItem {
    int resource_no;
    int attempt;
    std::string endpoint;
    TaskState state;
    time_t alloc_time;
    bool is_map;
};

struct AllocateItemComparator {
    bool operator()(AllocateItem* const& litem, AllocateItem* const& ritem) const {
        return litem->alloc_time < ritem->alloc_time;
    }
};

class JobTracker {

public:
    JobTracker(MasterImpl* master, ::baidu::galaxy::Galaxy* galaxy_sdk,
               const JobDescriptor& job);
    virtual ~JobTracker();

    Status Start();
    Status Update(const std::string& priority, int map_capacity, int reduce_capacity);
    Status Kill();
    ResourceItem* AssignMap(const std::string& endpoint);
    IdItem* AssignReduce(const std::string& endpoint);
    Status FinishMap(int no, int attempt, TaskState state);
    Status FinishReduce(int no, int attempt, TaskState state);

    std::string GetJobId() {
        MutexLock lock(&mu_);
        return job_id_;
    }
    JobDescriptor GetJobDescriptor() {
        MutexLock lock(&mu_);
        return job_descriptor_;
    }
    JobState GetState() {
        MutexLock lock(&mu_);
        return state_;
    }
    TaskStatistics GetMapStatistics();
    TaskStatistics GetReduceStatistics();

    template <class Inserter>
    Status Check(Inserter& task_inserter) {
        MutexLock lock(&alloc_mu_);
        for (std::list<AllocateItem*>::iterator it = allocation_table_.begin();
                it != allocation_table_.end(); ++it) {
            TaskOverview task;
            TaskInfo* info = task.mutable_info();
            info->set_task_id((*it)->resource_no);
            info->set_attempt_id((*it)->attempt);
            info->set_task_type((job_descriptor_.job_type() == kMapOnlyJob) ? kMapOnly :
                    ((*it)->is_map ? kMap : kReduce));
            ResourceItem* const res = map_manager_->CheckCertainItem((*it)->resource_no);
            TaskInput* input = info->mutable_input();
            input->set_input_file(res->input_file);
            input->set_input_offset(res->offset);
            input->set_input_size(res->size);
            task.set_state((*it)->state);
            task.set_minion_addr((*it)->endpoint);
            task_inserter = task;
        }
        return kOk;
    }

private:
    void KeepMonitoring();

private:
    MasterImpl* master_;
    ::baidu::galaxy::Galaxy* sdk_;
    Mutex mu_;
    JobDescriptor job_descriptor_;
    std::string job_id_;
    JobState state_;
    // Resource allocation
    Mutex alloc_mu_;
    std::list<AllocateItem*> allocation_table_;
    std::priority_queue<AllocateItem*, std::vector<AllocateItem*>,
                        AllocateItemComparator> time_heap_;
    // Map resource
    std::string map_minion_;
    ::baidu::galaxy::JobDescription map_description_;
    ResourceManager* map_manager_;
    int map_completed_;
    int last_map_no_;
    int last_map_attempt_;
    // Reduce resource
    std::string reduce_minion_;
    ::baidu::galaxy::JobDescription reduce_description_;
    BasicManager* reduce_manager_;
    int reduce_completed_;
    int last_reduce_no_;
    int last_reduce_attempt_;
    // Thread for monitoring
    ThreadPool monitor_;
    // To communicate with minion
    RpcClient* rpc_client_;
    // To check if output path is exists
    FileSystem* fs_;
};

}
}

#endif

