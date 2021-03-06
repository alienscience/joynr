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
#ifndef LOCALCAPABILITIESCALLBACKWRAPPER_H
#define LOCALCAPABILITIESCALLBACKWRAPPER_H
#include "joynr/PrivateCopyAssign.h"

#include "cluster-controller/capabilities-client/IGlobalCapabilitiesCallback.h"
#include "common/InterfaceAddress.h"
#include "joynr/DiscoveryQos.h"

#include <QSharedPointer>
#include <QString>

namespace joynr {

class ILocalCapabilitiesCallback;
class LocalCapabilitiesDirectory;

class LocalCapabilitiesCallbackWrapper : public IGlobalCapabilitiesCallback {
public:
    LocalCapabilitiesCallbackWrapper(LocalCapabilitiesDirectory* localCapabilitiesDirectory, QSharedPointer<ILocalCapabilitiesCallback> wrappedCallback,const QString &participantId, const DiscoveryQos& discoveryQos);
    LocalCapabilitiesCallbackWrapper(LocalCapabilitiesDirectory* localCapabilitiesDirectory, QSharedPointer<ILocalCapabilitiesCallback> wrappedCallback,const InterfaceAddress& interfaceAddress, const DiscoveryQos& discoveryQos);

    void capabilitiesReceived(QList<types::CapabilityInformation> results);
private:
    LocalCapabilitiesDirectory* localCapabilitiesDirectory;
    QSharedPointer<ILocalCapabilitiesCallback> wrappedCallback;
    QString participantId;
    InterfaceAddress interfaceAddress;
    DiscoveryQos discoveryQos;

    DISALLOW_COPY_AND_ASSIGN(LocalCapabilitiesCallbackWrapper);
};


} // namespace joynr
#endif //LOCALCAPABILITIESCALLBACKWRAPPER_H
