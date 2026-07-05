#pragma once

namespace severance::core::events {

enum class EventType {
  None = 0,
  ProcessCreated,
  ProcessTerminated,
  NetworkConnectionOpened,
  NetworkConnectionClosed,
  FileCreated,
  FileModified,
  FileDeleted,
  AppQuit,
  MacrodataRefined
};

} // namespace severance::core::events
