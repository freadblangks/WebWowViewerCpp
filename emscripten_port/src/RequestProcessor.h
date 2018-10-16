//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H

#include "../../wowViewerLib/src/include/wowScene.h"
#include <thread>
#include <list>
#include <vector>
#include <forward_list>

class RequestProcessor : public IFileRequest {
protected:
    RequestProcessor() {

    }

protected:
    IFileRequester *m_fileRequester = nullptr;

    virtual void processFileRequest(std::string &fileName, CacheHolderType holderType) = 0;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }

private:
    struct RequestStruct {
        std::string fileName;
        CacheHolderType holderType;
    };

    struct ResultStruct {
        std::string fileName;
        CacheHolderType holderType;
        std::vector<unsigned char> buffer;
    };

    std::thread *loaderThread;

    std::list<RequestStruct> m_requestQueue;
    std::list<ResultStruct> m_resultQueue;

    bool m_threaded = false;

public:
    void processResults(int limit);
    void processRequests(bool calledFromThread);
    void provideResult(std::string &fileName, std::vector<unsigned char> &content, CacheHolderType holderType);

    void setThreaded(bool value) {
        m_threaded = value;
        if (value) {
            loaderThread = new std::thread(([&](){
                this->processRequests(true);
            }));
        }
    }
    int currentlyProcessing = 0;
protected:
    void addRequest (std::string &fileName, CacheHolderType holderType);


};

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
