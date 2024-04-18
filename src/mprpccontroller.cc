#include "mprpccontroller.h"

MprpcController::MprpcController()
{
    m_failed = false;
    m_errText = "";
}

void MprpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool MprpcController::Failed() const
{
    return m_failed;
}

std::string MprpcController::ErrorText() const
{
    return m_errText;
}

void MprpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}

void MprpcController::StartCancel(){
    canceled_ = true;
}
bool MprpcController::IsCanceled() const {
     return canceled_;
}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback) {
    if (canceled_) {
            callback->Run();
    } 
    else {
        // 将回调函数保存起来，在取消 RPC 调用时触发
        cancel_callbacks_.push_back(callback);
    }
}