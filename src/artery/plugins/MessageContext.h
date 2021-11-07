/********************************************************************************
  \project  INFM_HIL_Interface
  \file     MessageContext.h
  \brief    Provides the class for managing the context in which the messages are sent
  \author   Alexander Pilz
  \version  1.0.0
  \date     04.11.2021
 ********************************************************************************/

#ifndef ARTERY_MESSAGECONTEXT_H
#define ARTERY_MESSAGECONTEXT_H

/********************************************************************************
 * Includes
 *********************************************************************************/

#include <zmq.hpp>
#include <unordered_map>
#include <iostream>


/********************************************************************************
 * Class declaration
 ********************************************************************************/

class MessageContext
        {
        public:
            using ContextName = std::string;

            zmq::context_t & AddContext(ContextName name, int io_threads_);
            zmq::context_t & GetContext(ContextName name);

        private:
            std::unordered_map<ContextName,zmq::context_t*> contextMap_;
};



#endif //ARTERY_MESSAGECONTEXT_H

/********************************************************************************
 * EOF
 ********************************************************************************/