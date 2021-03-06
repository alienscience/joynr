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
#ifndef TRACKABLEOBJECT_H
#define TRACKABLEOBJECT_H

#include "joynr/JoynrCommonExport.h"
#include "joynr/joynrlogging.h"


namespace joynr {

class JOYNRCOMMON_EXPORT TrackableObject
{
public:
    TrackableObject();
    virtual ~TrackableObject();
    static int getInstances();
private:
    static int instances;
    static joynr_logging::Logger* logger;

};


} // namespace joynr
#endif // TRACKABLEOBJECT_H
