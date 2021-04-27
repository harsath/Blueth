#pragma once
#include "io/IOBuffer.hpp"
#include <functional>
#include <memory>

namespace blueth::net {

enum class StreamType {
	SSLSyncStream,
	SSLAsyncStream,
	SyncStream,
	AsyncStream
};

enum class StreamMode { Client, Server };

enum class StreamProtocol { TCP, UDP };

/**
 * A stream is a reliable data transfer channel of bytes between endpoints and
 * it makes suer Wsure that the bytes are read and written in the order given in
 * the blueth::io::IOBuffer<BufferType> It's a very basic abstraction for
 * writing a client/server application
 */

/**
 * BufferType can be a char or unsigned char, It's a 8 bit octet.
 * ReadCallback is a optional feature in which we invoke the Callable object
 * when we read a bytes off the wire WriteCallback is an optional feature in
 * which we invoke the given Callable object after we written all the bytes from
 * the IOBuffer These optional feature can be set with a boolean value from the
 * invokeCallbacks method.
 */
template <typename BufferType> class NetworkStream {
      public:
	using buffer_type = std::unique_ptr<io::IOBuffer<BufferType>>;
	using const_buffer_type = std::unique_ptr<io::IOBuffer<BufferType>>;
	using buffer_reference_type =
	    std::unique_ptr<io::IOBuffer<BufferType>> &;
	using const_buffer_reference_type =
	    const std::unique_ptr<io::IOBuffer<BufferType>> &;

	/**
	 * Read raw bytes into the io_buffer from the network wire.
	 *
	 * This virtual method is used when we want to read some bytes from an
	 * endpoint which is set from during the construction of the instance of
	 * this interface. The bytes are read into the underlying IOBuffer
	 * object, which a client can get from getIOBuffer method.
	 *
	 * If we invoke the streamRead method with an invalid internal IOBuffer
	 * object, an exception of type std::runtime_error is thrown
	 *
	 * @param read_length Read length parameter
	 * by which we can specify the number bytes we need to read from an
	 * endpoint. This is mainly used to see if the bytes capacity of the
	 * IOBuffer is high enough to hold the request bytes read from wire. If
	 * there is less space on the IOBuffer than we need to read, we increase
	 * the memory capacity of the IOBuffer
	 * @return Returns number of bytes read on sucess and negative value on
	 * failur. We assume that a user had a logic to handle the failur in a
	 * graceful way on their logic
	 */
	virtual int streamRead(size_t read_length) noexcept(false) = 0;
	/**
	 * Write raw bytes from IOBuffer onto the endpoint on wire
	 *
	 * The IOBuffer from which we write the byte are either set through
	 * setIOBuffer method or it's the bytes which are read from the wire and
	 * done some operations on it with the inserted callbacks.
	 *
	 * If we invoke the streamWrite on an invalid IOBuffer or moved IOBuffer
	 * after a user called getIOBuffer but didn't set a buffer for
	 * reading-into before invoking this method, an exception of type
	 * std::runtime_error is thrown
	 *
	 * @param data Write the data in the std::string onto the endpoint.
	 * Internally the data is moved to a IOBuffer before sending off the
	 * wire
	 * @return Returns number of bytes written on success and a negative
	 * value if the operation fails. We assume that a user had a logic to
	 * handle the failur in a graceful way
	 */
	virtual int streamWrite(const std::string &data) noexcept(false) = 0;
	// TODO: Maybe use std::string_view for the 'data'? since we only need
	// Ptr and Size of memory
	/**
	 * Flush the internal IOBuffer object's data/contents but the object
	 * itself is not destroyed and the capaciy is same as the lasttime,
	 * except the buffer is empty or doesn't hold any valid data since it's
	 * "fresh" once more.
	 *
	 * If we make this operation on an invalid IOBuffer, which is either
	 * moved somewhere explicitly by the user when we called getIOBuffer, an
	 * exception of type std::runtime_error is thrown
	 */
	virtual void flushBuffer() noexcept(false) = 0;
	/**
	 * Get the internal IOBuffer and move the ownership to the caller. After
	 * this operation, the IOBuffer moved the ownership to the caller.
	 *
	 * If we try to do this operation on an already moved buffer or on an
	 * invalid buffer, an exception of type std::runtime_error is thrown
	 *
	 * @return Unique pointer of IOBuffer<BufferType> is returned if the
	 * object stores a valid buffer.
	 */
	virtual buffer_type getIOBuffer() noexcept = 0;
	/**
	 * Get a Const-Ref Unique pointer to IOBuffer. This call does not modify
	 * or transfer the ownership of the underlying IOBuffer object
	 *
	 * We throw an exception of type std::runtime_error if someone tries to
	 * call this method after they invalidated the buffer either throw
	 * calling getIOBuffer, since we moved the ownership.
	 *
	 * @return Const version of the underlying IOBuffer
	 */
	virtual const_buffer_reference_type constGetIOBuffer() const
	    noexcept(false) = 0;
	/**
	 * Set a io_buffer object as an underlying buffer for reading/writing
	 * onto the network.
	 *
	 * We need to invoke this method with a valid IOBuffer when previously
	 * transfered the ownership with getIOBuffer call or invalidated the
	 * underlying buffer. If we invoke read/write operations on an
	 * invalid(or ownership transfered) buffer, an exception of type
	 * std::runtime_error is thrown.
	 * @param io_buffer The IOBuffer object, which we need to set as the
	 * internal member
	 */
	virtual void setIOBuffer(buffer_type io_buffer) noexcept = 0;
	/**
	 * Get the StreamType which this instance supportes.
	 *
	 * @return Value of type StreamType
	 */
	virtual StreamType getStreamType() const noexcept = 0;
	/**
	 * Get the current stream mode to determins if this instance is acting
	 * as a server or client.
	 *
	 * @return Value of type StreamMode
	 */
	virtual StreamMode getStreamMode() const noexcept = 0;
	/**
	 * Get the stream protocol(TCP or UDP) the instance of this interface
	 * supports
	 *
	 * @return Value of type StreamProtocol
	 */
	virtual StreamProtocol getStreamProtocol() const noexcept = 0;
	/**
	 * Set a Callable object which needs to be optionally invoked when we
	 * read some bytes into IOBuffer from wire.
	 *
	 * This is ideally an object with operator()() overload or std::function
	 * or a lambda or a function pointer. The signacture of the Callable
	 * must return void and takes a parameter of type const
	 * std::unique_ptr<io::IOBuffer<BufferType>>& or
	 * const_buffer_reference_type
	 * @param callable Callable object to be invoked by passing the
	 * IOBuffer.
	 */
	virtual void
	setReadCallback(std::function<void(const_buffer_reference_type)>
			    callable) noexcept = 0;
	/**
	 * Set a Callable object which needs to be optionally invoked when we
	 * write bytes from the IOBuffer onto the endpoint. We invoke it after
	 * we written all the bytes on the wire
	 *
	 * This is ideally an object with operator()() overload or std::function
	 * or a lambda or a function pointer. The signacture of the Callable
	 * must return void and takes a parameter of type const
	 * std::unique_ptr<io::IOBuffer<BufferType>>& or
	 * const_buffer_reference_type
	 * @param callable Callable object to be invoked by passing the
	 * IOBuffer.
	 */
	virtual void
	setWriteCallback(std::function<void(const_buffer_reference_type)>
			     callable) noexcept = 0;
	/**
	 * Get the internal callback function which is registered as the read
	 * callback
	 *
	 * @return std::function object callback
	 */
	virtual std::function<void(const_buffer_reference_type)>
	getReadCallback() const noexcept = 0;
	/**
	 * Get the internal callback function which is registered as the write
	 * callback
	 *
	 * @return std::function object callback
	 */
	virtual std::function<void(const_buffer_reference_type)>
	getWriteCallback() const noexcept = 0;
	/**
	 * Close the network connection with the peer
	 *
	 * If an error occured during the operation, an exception of type
	 * std::runtime_error is thrown
	 */
	virtual void closeConnection() noexcept(false) = 0;
	virtual ~NetworkStream() = default;
};

} // namespace blueth::net
