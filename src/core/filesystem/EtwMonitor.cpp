#include "EtwMonitor.hpp"
#include "logging/Logger.hpp"
#include "events/EventBus.hpp"
#include "events/EventTypes.hpp"

#ifdef _WIN32
#include <windows.h>
#include <evntcons.h>
#include <evntrace.h>
#include <tdh.h>
#pragma comment(lib, "tdh.lib")

#include <thread>
#include <vector>

namespace severance::core::filesystem {

// Note: ETW tracing for Microsoft-Windows-Kernel-File requires Administrator privileges.
// If not admin, StartTrace will return ERROR_ACCESS_DENIED.

static const GUID SystemTraceControlGuid = 
    {0x9e814aad, 0x3204, 0x11d2, {0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39}};

void WINAPI EtwMonitor::EventRecordCallback(PEVENT_RECORD pEventRecord) {
  EtwMonitor::GetInstance().ProcessEvent(pEventRecord);
}

bool EtwMonitor::Start() {
  if (m_Running) return true;

  // 1. Configure trace properties
  ULONG bufferSize = sizeof(EVENT_TRACE_PROPERTIES) + m_SessionName.length() + 1;
  std::vector<uint8_t> buffer(bufferSize, 0);
  auto* pSessionProperties = reinterpret_cast<PEVENT_TRACE_PROPERTIES>(buffer.data());

  pSessionProperties->Wnode.BufferSize = bufferSize;
  pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
  pSessionProperties->Wnode.ClientContext = 1; // QPC timer
  pSessionProperties->Wnode.Guid = SystemTraceControlGuid;
  
  pSessionProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
  pSessionProperties->MaximumFileSize = 0; // Not logging to file
  pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
  
  // Enable File IO tracing flags
  pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_DISK_FILE_IO | EVENT_TRACE_FLAG_FILE_IO | EVENT_TRACE_FLAG_FILE_IO_INIT;

  // 2. Start trace session
  // Stop existing if any
  ControlTraceA((TRACEHANDLE)0, m_SessionName.c_str(), pSessionProperties, EVENT_TRACE_CONTROL_STOP);

  ULONG status = StartTraceA(&m_SessionHandle, m_SessionName.c_str(), pSessionProperties);
  if (status != ERROR_SUCCESS) {
    if (status == ERROR_ACCESS_DENIED) {
      SEV_CORE_ERROR("ETW StartTrace failed: Access Denied. Severance must run as Administrator to capture File events.");
    } else {
      SEV_CORE_ERROR("ETW StartTrace failed with error code: {}", status);
    }
    return false;
  }

  // 3. Open trace for consumption
  EVENT_TRACE_LOGFILEA logFile = {0};
  logFile.LoggerName = const_cast<char*>(m_SessionName.c_str());
  logFile.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
  logFile.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EtwMonitor::EventRecordCallback;

  m_TraceHandle = OpenTraceA(&logFile);
  if (m_TraceHandle == INVALID_PROCESSTRACE_HANDLE) {
    SEV_CORE_ERROR("ETW OpenTrace failed.");
    ControlTraceA(m_SessionHandle, m_SessionName.c_str(), pSessionProperties, EVENT_TRACE_CONTROL_STOP);
    return false;
  }

  m_Running = true;
  SEV_CORE_INFO("ETW File Monitor started successfully.");

  // 4. Start processing thread
  std::thread([this]() {
    ULONG status = ProcessTrace(&m_TraceHandle, 1, 0, 0);
    if (status != ERROR_SUCCESS && status != ERROR_CANCELLED) {
      SEV_CORE_ERROR("ETW ProcessTrace failed: {}", status);
    }
  }).detach();

  return true;
}

void EtwMonitor::Stop() {
  if (!m_Running) return;

  m_Running = false;

  ULONG bufferSize = sizeof(EVENT_TRACE_PROPERTIES) + m_SessionName.length() + 1;
  std::vector<uint8_t> buffer(bufferSize, 0);
  auto* pSessionProperties = reinterpret_cast<PEVENT_TRACE_PROPERTIES>(buffer.data());
  pSessionProperties->Wnode.BufferSize = bufferSize;
  pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

  ControlTraceA(m_SessionHandle, m_SessionName.c_str(), pSessionProperties, EVENT_TRACE_CONTROL_STOP);
  CloseTrace(m_TraceHandle);
  
  m_SessionHandle = 0;
  m_TraceHandle = 0;
  SEV_CORE_INFO("ETW File Monitor stopped.");
}

void EtwMonitor::ProcessEvent(PEVENT_RECORD pEventRecord) {
  if (!m_Callback) return;

  // We are interested in FileIo events (Guid: 90cbdc39-4a3e-11d1-84f4-0000f80464e3)
  // Opcodes: 64 (Create), 65 (Cleanup), 66 (Close), 67 (Read), 68 (Write), etc.
  // For simplicity, we just format basic info, but full parsing requires TDH.

  const auto& providerId = pEventRecord->EventHeader.ProviderId;
  static const GUID FileIoGuid = {0x90cbdc39, 0x4a3e, 0x11d1, {0x84, 0xf4, 0x00, 0x00, 0xf8, 0x04, 0x64, 0xe3}};

  if (memcmp(&providerId, &FileIoGuid, sizeof(GUID)) == 0) {
    uint8_t opcode = pEventRecord->EventHeader.EventDescriptor.Opcode;
    
    FileEvent fe;
    fe.timestamp = pEventRecord->EventHeader.TimeStamp.QuadPart;
    fe.pid = pEventRecord->EventHeader.ProcessId;
    
    switch (opcode) {
      case 64: fe.operation = "Create"; break;
      case 67: fe.operation = "Read"; break;
      case 68: fe.operation = "Write"; break;
      case 70: fe.operation = "SetInfo"; break;
      case 71: fe.operation = "Delete"; break;
      case 72: fe.operation = "Rename"; break;
      case 73: fe.operation = "DirEnum"; break;
      case 74: fe.operation = "Flush"; break;
      default: return; // Ignore others
    }

    // Getting the filename via TDH is expensive and requires GetEventInformation.
    // For this prototype, we pass raw ETW event parsing using basic buffer offset assumption (which is brittle),
    // or we just emit the PID + Opcode for now to prove Correlation Engine connectivity.
    fe.filePath = "<Requires TDH parsing logic>";
    fe.processName = "PID: " + std::to_string(fe.pid);

    m_Callback(fe);
  }
}

} // namespace severance::core::filesystem
#else
namespace severance::core::filesystem {
bool EtwMonitor::Start() { return false; }
void EtwMonitor::Stop() {}
void EtwMonitor::ProcessEvent(void*) {}
void EtwMonitor::EventRecordCallback(void*) {}
}
#endif
