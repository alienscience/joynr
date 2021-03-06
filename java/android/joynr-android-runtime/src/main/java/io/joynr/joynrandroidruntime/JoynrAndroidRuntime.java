package io.joynr.joynrandroidruntime;

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

import io.joynr.capabilities.RegistrationFuture;
import io.joynr.dispatcher.rpc.JoynrInterface;
import io.joynr.exceptions.JoynrIllegalStateException;
import io.joynr.provider.JoynrProvider;
import io.joynr.proxy.ProxyBuilder;
import io.joynr.runtime.JoynrRuntime;

import java.util.concurrent.ExecutionException;

import android.content.Context;
import android.os.Messenger;

public class JoynrAndroidRuntime implements JoynrRuntime {

    private InitRuntimeTask runtimeInitTask;
    private static UILogger uiLogger = new UILogger();

    public JoynrAndroidRuntime(Context applicationContext) {
        runtimeInitTask = new InitRuntimeTask(applicationContext, uiLogger);
        runtimeInitTask.execute();
    }

    private JoynrRuntime getJoynrRuntime() {
        // this will block until the runtime is created successfully
        // TODO since the caller expects the register call to be async, we need to check if
        // this will not block to long 
        JoynrRuntime runtime;
        try {
            runtime = runtimeInitTask.get();
        } catch (InterruptedException e) {
            e.printStackTrace();
            uiLogger.logText(e.getMessage());
            return null;
        } catch (ExecutionException e) {
            e.printStackTrace();
            uiLogger.logText(e.getMessage());
            return null;
        }
        return runtime;
    }

    @Override
    public <T extends JoynrInterface> RegistrationFuture registerCapability(String domain,
                                                                            JoynrProvider provider,
                                                                            Class<T> providedInterface,
                                                                            String authenticationToken) {
        // this will block until the runtime is created successfully
        // TODO since the caller expects the register call to be async, we need to check if
        // this will not block to long 
        JoynrRuntime runtime = getJoynrRuntime();

        // registration of providers is asynchronously
        RegistrationFuture future = runtime.registerCapability(domain, provider, providedInterface, authenticationToken);

        return future;
    }

    @Override
    public <T extends JoynrInterface> void unregisterCapability(String domain,
                                                                JoynrProvider provider,
                                                                Class<T> providedInterface,
                                                                String autheticationToken) {
        // this will block until the runtime is created successfully
        // TODO since the caller expects the unregister call to be async, we need to check if
        // this will not block to long 
        JoynrRuntime runtime = getJoynrRuntime();

        runtime.unregisterCapability(domain, provider, providedInterface, autheticationToken);
    }

    @Override
    public <T extends JoynrInterface> ProxyBuilder<T> getProxyBuilder(String domain, Class<T> interfaceClass)
                                                                                                             throws JoynrIllegalStateException {
        return new AndroidProxyBuilder<T>(runtimeInitTask, domain, interfaceClass, uiLogger);
    }

    public void addLogListener(Messenger clientMessenger) {
        uiLogger.addLogListener(clientMessenger);
    }

    public void removeLogListener(Messenger clientMessanger) {
        uiLogger.removeLogListener(clientMessanger);
    }

    @Override
    public void shutdown(boolean clear) {
        // TODO implement shutdown method for JoynAndroidRuntime

    }

}
