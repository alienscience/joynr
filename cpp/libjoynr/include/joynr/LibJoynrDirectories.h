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
#ifndef LIBJOYNRDIRECTORIES_H
#define LIBJOYNRDIRECTORIES_H

#include "joynr/RequestCaller.h"
#include "joynr/IReplyCaller.h"
#include "joynr/Directory.h"
#include "joynr/ISubscriptionCallback.h"

namespace joynr {

    typedef Directory< QString, RequestCaller > RequestCallerDirectory;
    typedef Directory< QString, IReplyCaller > ReplyCallerDirectory;
    typedef Directory< QString, ISubscriptionCallback > AttributeSubscriptionDirectory;


} // namespace joynr
#endif //LIBJOYNRDIRECTORIES_H
