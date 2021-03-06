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
#include "common/dbus/DbusMessagingStubAdapter.h"
#include "common/dbus/DbusMessagingUtil.h"
#include <CommonAPI/CommonAPI.h>

namespace joynr {

using namespace joynr_logging;

DbusMessagingStubAdapter::DbusMessagingStubAdapter(QString serviceAddress):
    IDbusStubWrapper(serviceAddress)
{
    // init logger
    logger = Logging::getInstance()->getLogger("MSG", "DbusMessagingStubAdapter");
    LOG_INFO(logger, "Get dbus proxy on address: " + serviceAddress);

    // init the stub
    init();
}

void DbusMessagingStubAdapter::transmit(JoynrMessage &message, const MessagingQos &qos) {
    logMethodCall("transmit");
    // copy joynr message
    joynr::messaging::IMessaging::JoynrMessage dbusMsg;
    DbusMessagingUtil::copyJoynrMsgToDbusMsg(message, dbusMsg);
    // copy qos
    joynr::messaging::types::Types::JoynrMessageQos dbusQos;
    DbusMessagingUtil::copyJoynrQosToDbusQos(qos, dbusQos);
    // call
    CommonAPI::CallStatus status;
    proxy->transmit(dbusMsg, dbusQos, status);
    // print the status
    printCallStatus(status, "transmit");
}

} // namespace joynr
