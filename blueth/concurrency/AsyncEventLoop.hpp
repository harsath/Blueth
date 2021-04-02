#pragma once
#include "internal/EventLoopBase.hpp"

namespace blueth::concurrency {

template <typename PeerStatePtr>
class AsyncEventLoop final : public EventLoopBase<PeerStatePtr> {
      public:
	AsyncEventLoop();
	void register_callback_accept(
	    std::function<FDStatus(PeerStatePtr)>) override;
	void register_callback_event(std::function<FDStatus(PeerStatePtr)>) override;
	virtual void remove_callback_event(PeerStatePtr, event_type) override;
};



} // namespace blueth::concurrency
