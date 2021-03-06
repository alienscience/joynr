package io.joynr.capabilities;

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

import io.joynr.dispatcher.rpc.JoynrInterface;
import io.joynr.provider.JoynrProvider;

public interface CapabilitiesRegistrar {

    /**
     * Registers a provider at the capabilities directory to make it available at other cluster controllers and the
     * messaging endpoint directory to dispatch incoming requests.
     * 
     * @param domain
     *            Domain of the provided service.
     * @param provider
     *            Provider instance.
     * @param providedInterface
     *            Interface class which is implemented by the provider and should be accessible by proxies. The provider
     *            only has to implement the sync interface.
     * @return
     */
    <T extends JoynrInterface> RegistrationFuture registerCapability(String domain,
                                                                     JoynrProvider provider,
                                                                     Class<T> providedInterface,
                                                                     String authenticationToken);

    <T extends JoynrInterface> void unregisterCapability(String domain,
                                                         JoynrProvider provider,
                                                         Class<T> providedInterface,
                                                         String authenticationToken);

    /**
     * Shuts down the local capabilities directory and all used thread pools.
     * @param unregisterAllRegisteredCapabilities if set to true, all added capabilities that are not removed up to
     * this point, will be removed automatically 
     */
    void shutdown(boolean unregisterAllRegisteredCapabilities);
}
