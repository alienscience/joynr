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
#include "MockLocalCapabilitiesDirectoryCallback.h"
#include "utils/QThreadSleep.h"

using namespace joynr;

MockLocalCapabilitiesDirectoryCallback::MockLocalCapabilitiesDirectoryCallback()
    : ILocalCapabilitiesCallback(),
      results(),
      semaphore(1) {
    semaphore.acquire();
}

void MockLocalCapabilitiesDirectoryCallback::capabilitiesReceived(QList<CapabilityEntry> capabilities) {
    this->results = capabilities;
    semaphore.release();
}

QList<CapabilityEntry> MockLocalCapabilitiesDirectoryCallback::getResults(int timeout) {
    const int waitInterval = 20;
    for (int i = 0; i < timeout; i += waitInterval) {
        QThreadSleep::msleep(waitInterval);
        if (semaphore.tryAcquire()) {
            semaphore.release();
            return results;
        }
    }

    return results;
}

void MockLocalCapabilitiesDirectoryCallback::clearResults(){
    results.clear();
    semaphore.tryAcquire(1);
}

MockLocalCapabilitiesDirectoryCallback::~MockLocalCapabilitiesDirectoryCallback() {
    results.clear();
}
