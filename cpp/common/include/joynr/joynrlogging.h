/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
 * %%
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * #L%
 */
#ifndef JOYNRLOGGING_H_
#define JOYNRLOGGING_H_

#include "joynr/joynrloggingmacros.h"
#include "joynr/JoynrCommonExport.h"

#define NOGDI

class QString;

namespace joynr {

namespace joynr_logging {

class Logger;

#ifdef WIN32
  #ifdef ERROR
    // QT5.1 leaks this global definition from windows.h
    // Because Joynr does not use windows.h directly it is safe to 
    // undefine this macro
    #undef ERROR
  #endif
#endif

enum LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};


/**
  * Singleton class that represents the logging system.
  * Use Logging::getInstance()->getLogger("MSG", "MyClass"); to get a logger for the class named MyClass.
  * MSG would be the module id, that must not be longer than four characters.
  */
class JOYNRCOMMON_EXPORT Logging {
public:
    virtual ~Logging() {};
    virtual void shutdown() = 0;

    virtual Logger* getLogger(QString contextId, QString className) = 0;
    virtual void destroyLogger(QString contextId, QString className) = 0;

    static Logging* getInstance();
};


/**
  * Objects of this class are used to perform the actual logging.
  * Instead of calling the log method of this class you should concider using the logging macros:
  * LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR and LOG_FATAL.
  * An example would be LOG_INFO(logger, "connected to the server").
  * The macros have the benefit, that they do not evaluate to code if JOYNR_MAX_LOG_LEVEL_XXXX macro is defined.
  */
class JOYNRCOMMON_EXPORT Logger {
public:
    virtual ~Logger(){};
    virtual void log(LogLevel logLevel, const char* message) = 0;
    virtual void log(LogLevel logLevel, const QString& message) = 0;
};


} // namespace joynr
} // namespace joynr_logging

#endif // JOYNRLOGGING_H_
