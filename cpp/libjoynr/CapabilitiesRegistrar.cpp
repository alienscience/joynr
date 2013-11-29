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
#include "joynr/CapabilitiesRegistrar.h"
#include "joynr/ParticipantIdStorage.h"

namespace joynr {

CapabilitiesRegistrar::CapabilitiesRegistrar(QList<IDispatcher*> dispatcherList,
                                             QSharedPointer<ICapabilities> capabilitiesAggregator,
                                             QSharedPointer<EndpointAddressBase> messagingStubAddress,
                                             QSharedPointer<ParticipantIdStorage> participantIdStorage)
    : dispatcherList(dispatcherList),
      capabilitiesAggregator(capabilitiesAggregator),
      messagingStubAddress(messagingStubAddress),
      participantIdStorage(participantIdStorage)
{

}

void CapabilitiesRegistrar::unregisterCapability(QString participantId){
    foreach (IDispatcher* currentDispatcher, dispatcherList) {
        currentDispatcher->removeRequestCaller(participantId);
    }
    capabilitiesAggregator->remove(participantId, ICapabilities::NO_TIMEOUT());
}

void CapabilitiesRegistrar::addDispatcher(IDispatcher* dispatcher){
    dispatcherList.append(dispatcher);
}

void CapabilitiesRegistrar::removeDispatcher(IDispatcher* dispatcher){
    dispatcherList.removeAll(dispatcher);
}

} // namespace joynr
