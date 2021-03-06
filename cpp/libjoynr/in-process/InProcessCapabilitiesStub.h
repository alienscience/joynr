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
#ifndef DUMMYCAPABILITIESSTUB_H
#define DUMMYCAPABILITIESSTUB_H
#include "joynr/PrivateCopyAssign.h"

#include "joynr/JoynrExport.h"
#include "joynr/ICapabilities.h"

namespace joynr {

class JOYNR_EXPORT InProcessCapabilitiesStub : public ICapabilities {
public:
    InProcessCapabilitiesStub(ICapabilities* skeleton);
    void add(
            const QString &domain,
            const QString &interfaceName,
            const QString &participantId,
            const types::ProviderQos &qos,
            QList<QSharedPointer<joynr::system::Address> > endpointAddressList,
            QSharedPointer<joynr::system::Address> messagingStubAddress,
            const qint64& timeout_ms
    );
    void addEndpoint(
            const QString &participantId,
            QSharedPointer<joynr::system::Address> messagingStubAddress,
            const qint64& timeout_ms
    );
    QList<CapabilityEntry> lookup(
            const QString &domain,
            const QString &interfaceName,
            const DiscoveryQos& discoveryQos
    );
    QList<CapabilityEntry> lookup(
            const QString& participantId,
            const DiscoveryQos& discoveryQos
    );
    void remove(const QString& participantId, const qint64& timeout_ms);
private:
    DISALLOW_COPY_AND_ASSIGN(InProcessCapabilitiesStub);
    ICapabilities* skeleton;
};


} // namespace joynr
#endif //DUMMYCAPABILITIESSTUB_H
