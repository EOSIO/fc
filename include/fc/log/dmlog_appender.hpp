#pragma once
#include <fc/log/appender.hpp>

namespace fc {

   class dmlog_appender : public appender
   {
       public:
            dmlog_appender( const variant& args );
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
