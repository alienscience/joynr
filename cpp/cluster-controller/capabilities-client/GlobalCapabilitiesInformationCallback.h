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
#ifndef GLOBALCAPABILITIESINFORMATIONCALLBACK_H
#define GLOBALCAPABILITIESINFORMATIONCALLBACK_H

#include "joynr/joynrlogging.h"
#include "joynr/ICallback.h"
#include "cluster-controller/capabilities-client/IGlobalCapabilitiesCallback.h"
#include "joynr/types/CapabilityInformation.h"

#include <QList>
#include <QSharedPointer>

namespace joynr {

/*
 * IGlobalCapabilitiesCallback is an old callback method for capabilities.
 * Now that Code is generated the new interface for callbacks using ICallback.h is needed
 * to access the proxy.
 * For this reason, we currently have two callbacks: One GlobalCapabilitiesInformationCallback : ICallback<types::CapabilityInformation> for the proxy
 * and one IGlobalCapabilitiesCallback for the application. The ICallback<types::CapabilityInformation> just calls the
 * IGlobalCapabilitiesCallback.
 * Those two callbacks should be merged into one.
 */


class GlobalCapabilitiesInformationCallback : public ICallback<QList<types::CapabilityInformation> > {
public:
    GlobalCapabilitiesInformationCallback(QSharedPointer<IGlobalCapabilitiesCallback> igc);
    virtual ~GlobalCapabilitiesInformationCallback();
    virtual void onFailure(const RequestStatus status);
    virtual void onSuccess(const RequestStatus status, QList<types::CapabilityInformation> result);
private:
    QSharedPointer<IGlobalCapabilitiesCallback> callback;
};

} // namespace joynr

#endif //CAPABILITIESRESULTCALLBACK_H
