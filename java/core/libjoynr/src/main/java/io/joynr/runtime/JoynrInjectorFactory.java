package io.joynr.runtime;

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

import java.util.Properties;

import com.google.inject.Inject;
import com.google.inject.Module;

public class JoynrInjectorFactory extends AbstractJoynrInjectorFactory {

    @Inject
    public JoynrInjectorFactory() {
        this(new Module[0]);
    }

    /**
     * Creates a {@code JoynInjectorFactory} with empty configuration properties.
     *
     * @param modules (optional parameter) for <b>joynr internal use only</b>
     */
    public JoynrInjectorFactory(Module... modules) {
        this(new Properties(), modules);
    }

    /**
     * Creates a {@code JoynInjectorFactory} that uses the specified joynr configuration properties
     * during object injection/creation.
     *
     * @param joynrProperties joynr configuration properties to be used by this injector factory
     * @param modules (optional parameter) for <b>joynr internal use only</b>
     */
    public JoynrInjectorFactory(Properties joynrProperties, Module... joynrModules) {
        super(new JoynrBaseModule(joynrProperties, joynrModules));
    }

    @Override
    public void updateInjectorModule(Properties customJoynProperties, Module... modules) {
        updateModules(new JoynrBaseModule(customJoynProperties, modules));
    }

    @Override
    public JoynrApplication createApplication(JoynrApplicationModule applicationModule,
                                              Module... applicationSpecificModules) {
        return super.createApplication(applicationModule, applicationSpecificModules);
    }

    //TODO remove this and recompile and redeploy A4AInjectorFactory 
    public JoynrApplication createApplication(JoynrApplicationModule applicationModule) {
        return super.createApplication(applicationModule, (Module[]) null);
    }

    public JoynrApplication createApplication(Class<? extends JoynrApplication> applicationClass) {
        return super.createApplication(new JoynrApplicationModule(applicationClass));
    }

    public JoynrApplication createApplication(Class<? extends JoynrApplication> applicationClass,
                                              Properties applicationSpecificProperties) {
        return super.createApplication(new JoynrApplicationModule(applicationClass, applicationSpecificProperties));
    }

}
