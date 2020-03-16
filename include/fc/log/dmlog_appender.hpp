#pragma once
#include <fc/log/appender.hpp>

namespace fc {

   /**
    * Specialized appender for deep mind tracer that sends log messages
    * through `stdout` correctly formatted for latter consumption by
    * deep mind postprocessing tools from dfuse.
    */
   class dmlog_appender : public appender
   {
       public:
            explicit dmlog_appender( const variant& args );
            dmlog_appender();

            virtual ~dmlog_appender();
            void initialize( boost::asio::io_service& io_service ) {
                 setbuf(stdout, NULL);
            }

            virtual void log( const log_message& m );

       private:
            class impl;
            std::unique_ptr<impl> my;
   };
}
