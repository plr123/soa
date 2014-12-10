/* rest_connection.h                                               -*- C++ -*-
   Jeremy Barnes, 1 December 2014
   Copyright (c) 2014 Datacratic Inc.  All rights reserved.

   Connection object for REST connections.  Abstract class; can be specialized
   for the different kinds of connections.
*/

#pragma once

#include "soa/jsoncpp/json.h"
#include <memory>

namespace Datacratic {


/*****************************************************************************/
/* REST CONNECTION                                                           */
/*****************************************************************************/

/** Abstracts out a connection back to a REST service. */

struct RestConnection {

    virtual ~RestConnection()
    {
    }

    virtual void sendResponse(int responseCode,
                              const char * response,
                              const std::string & contentType)
    {
        return sendResponse(responseCode, std::string(response),
                            contentType);
    }
    
    /** Send the given response back on the connection. */
    virtual void sendResponse(int responseCode,
                              const std::string & response,
                              const std::string & contentType) = 0;
    
    /** Send the given response back on the connection. */
    virtual void
    sendResponse(int responseCode,
                 const Json::Value & response,
                 const std::string & contentType = "application/json") = 0;
    
    virtual void sendResponse(int responseCode)
    {
        return sendResponse(responseCode, "", "");
    }

    virtual void sendRedirect(int responseCode,
                              const std::string & location) = 0;

    /** Send an HTTP-only response with the given headers.  If it's not
        an HTTP connection, this will fail.
    */
    virtual void sendHttpResponse(int responseCode,
                                  const std::string & response,
                                  const std::string & contentType,
                                  const RestParams & headers) = 0;
    
    enum {
        UNKNOWN_CONTENT_LENGTH = -1,
        CHUNKED_ENCODING = -2
    };

    /** Send an HTTP-only response header.  This will not close the connection.  A
        contentLength of -1 means don't send it (for when the content length is
        not known ahead of time).  A contentLength of -2 means used HTTP chunked
        transfer encoding
    */
    virtual void
    sendHttpResponseHeader(int responseCode,
                           const std::string & contentType,
                           ssize_t contentLength,
                           const RestParams & headers = RestParams()) = 0;
    
    /** Send a payload (or a chunk of a payload) for an HTTP connection. */
    virtual void sendPayload(const std::string & payload) = 0;

    /** Finish the response, recycling or closing the connection. */
    virtual void finishResponse() = 0;

    /** Send the given error string back on the connection. */
    virtual void sendErrorResponse(int responseCode,
                                   const std::string & error,
                                   const std::string & contentType) = 0;
    
    virtual void sendErrorResponse(int responseCode, const char * error,
                                   const std::string & contentType)
    {
        sendErrorResponse(responseCode, std::string(error), "application/json");
    }

    virtual void sendErrorResponse(int responseCode, const Json::Value & error)
        = 0;

    virtual bool responseSent() const = 0;

    virtual bool isConnected() const = 0;

    /** Construct an object that captures this connection so that it can be
        written to asynchronously later.  It is obligatory to pass in an
        onDisconnect handler, which must have the direct result of destroying
        the connection when called.  The handler will be called if the
        connection is broken (from the other end) before the response is
        finished.

        Once capture() is called on a connection, it cannot be re-used (the
        captured connection must be used).  Note also that onDisconnect() will
        never be called from within the original handler function.
    */
    virtual std::shared_ptr<RestConnection> capture(std::function<void ()> onDisconnect) = 0;
};


} // namespace Datacratic