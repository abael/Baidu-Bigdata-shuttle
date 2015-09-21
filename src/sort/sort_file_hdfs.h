#ifndef _BAIDU_SHUTTLE_SORT_FILE_HDFS_H_
#define _BAIDU_SHUTTLE_SORT_FILE_HDFS_H_

#include "sort_file.h"
#include "hdfs.h"

#include <string>

namespace baidu {
namespace shuttle {
class SortFileHdfsReader : public SortFileReader {
public:
    class IteratorHdfs : public Iterator {
    public:
        IteratorHdfs(const std::string& start_key,
                     const std::string& end_key,
                     SortFileHdfsReader* reader);
        virtual ~IteratorHdfs();
        virtual bool Done();
        virtual void Next();
        virtual const std::string& Key();
        virtual const std::string& Value();
        virtual Status Error();
    private:
        SortFileHdfsReader* reader_;
    };

    virtual Status Open(const std::string& path, Param& param);
    virtual Iterator* Scan(const std::string& start_key, const std::string& end_key);
    virtual Status Close();
private:
    hdfsFS fs_;
    hdfsFile fd_;
};

class SortFileHdfsWriter : public SortFileWriter {
public:
    SortFileHdfsWriter();
    virtual Status Open(const std::string& path, Param& param);
    virtual Status Put(const std::string& key, const std::string& value);
    virtual Status Close();
private:
    Status FlushCurBlock();
    Status FlushIdxBlock();
    static bool KeyValueCmp(const KeyValue& a, const KeyValue& b);
    hdfsFS fs_;
    hdfsFile fd_;
    DataBlock cur_block_;
    IndexBlock idx_block_;
    int32_t cur_block_size_;
};

}
}

#endif

