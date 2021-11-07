/********************************************************************************
  \project  INFM_HIL_Interface
  \file     MessageContext.cpp
  \brief    Provides the functions for managing the context in which the messages are sent
  \author   Alexander Pilz
  \version  1.0.0
  \date     04.11.2021
 ********************************************************************************/


/********************************************************************************
 * Includes
 *********************************************************************************/

#include "MessageContext.h"
#include <zmq.hpp>
#include <unordered_map>
#include <iostream>

/********************************************************************************
 * Function declarations
 ********************************************************************************/

zmq::context_t &MessageContext::AddContext(MessageContext::ContextName name, int io_threads_) {
    zmq::context_t * context;

    //create a new context
    try {
        context = new zmq::context_t(io_threads_);

    }catch (std::exception& cantCreateContext) {

        std::cerr << "Can't create new context:" << cantCreateContext.what() << std::endl;
        throw cantCreateContext;
    }

    // Check if context name is already in use
    if(contextMap_.find(name) != contextMap_.end()) {
        delete(context);
        throw std::invalid_argument("Invalid context name");
    }

    // Insert the newly created context into a map for later reference
    auto rv = contextMap_.insert(std::make_pair(name, context));
    if(!rv.second) {
        delete(context);
        throw std::runtime_error("Failed to insert into unordered_map");
    }

    return *context;
}

zmq::context_t &MessageContext::GetContext(MessageContext::ContextName name) {
    auto iterator = contextMap_.find(name);
    if(iterator == contextMap_.end())
    {
        throw std::invalid_argument("Invalid context name");
    }

    return *iterator->second;
}
