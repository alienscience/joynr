package io.joynr.proxy;

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
import io.joynr.messaging.MessagingQos;

import java.lang.reflect.Proxy;

public final class ProxyFactory {

    private ProxyFactory() {

    }

    @SuppressWarnings("unchecked")
    // necessary for jenkins?
    public static <T extends JoynrInterface> T createProxy(Class<T> interfaceClass,
                                                           final MessagingQos qosSettings,
                                                           ProxyInvocationHandler proxyInvocationHandler) {
        return (T) Proxy.newProxyInstance(interfaceClass.getClassLoader(),
                                          new Class<?>[]{ interfaceClass },
                                          proxyInvocationHandler);
    }
}
