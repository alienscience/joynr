package io.joynr.demo;

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

import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.runtime.AbstractJoynrApplication;
import io.joynr.runtime.JoynrApplication;
import io.joynr.runtime.JoynrApplicationModule;
import io.joynr.runtime.JoynrInjectorFactory;

import java.util.Properties;

import joynr.vehicle.RadioProvider;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.inject.Module;

public class MyRadioProviderApplication extends AbstractJoynrApplication {
	private static final String AUTH_TOKEN = "MyRadioProvider_authToken";
	private static final Logger LOG = LoggerFactory
			.getLogger(MyRadioProviderApplication.class);
	public static final String STATIC_PERSISTENCE_FILE = "provider-joynr.properties";

	private MyRadioProvider provider = null;

	public static void main(String[] args) {
		// Get the provider domain from the command line
		if (args.length != 1) {
			LOG.error(
					"\n\nUSAGE: java {} <local-domain>\n\n NOTE: Providers are registered on the local domain.",
					MyRadioProviderApplication.class.getName());
			return;
		}
		String localDomain = args[0];
		LOG.debug("Registering provider on domain \"{}\"", localDomain);

		// joynr config properties are used to set joynr configuration at
		// compile time. They are set on the
		// JoynrInjectorFactory.
		Properties joynrConfig = new Properties();
		// Set a custom static persistence file (default is joynr.properties in
		// the working dir) to store
		// joynr configuration. It allows for changing the joynr configuration
		// at runtime. Custom persistence
		// files support running the consumer and provider applications from
		// within the same directory.
		joynrConfig.setProperty(MessagingPropertyKeys.PERSISTENCE_FILE,
				STATIC_PERSISTENCE_FILE);

		// How to use custom infrastructure elements:

		// 1) Set them programmatically at compile time using the joynr
		// configuration properties at the
		// JoynInjectorFactory. E.g. uncomment the following lines to set a
		// certain joynr server
		// instance.
		// joynrConfig.setProperty(MessagingPropertyKeys.BOUNCE_PROXY_URL,
		// "http://localhost:8080/bounceproxy/");
		// joynrConfig.setProperty(MessagingPropertyKeys.CAPABILITIESDIRECTORYURL,
		// "http://localhost:8080/bounceproxy/channels/discoverydirectory_channelid/");
		// joynrConfig.setProperty(MessagingPropertyKeys.CHANNELURLDIRECTORYURL,
		// "http://localhost:8080/bounceproxy/channels/discoverydirectory_channelid/");

		// Each joynr instance has a local domain. It identifies the execution
		// device/platform, e.g. the
		// vehicle. Normally, providers on that instance are registered for the
		// local domain.
		joynrConfig.setProperty(PROPERTY_JOYNR_DOMAIN_LOCAL, localDomain);

		// 2) Or set them in the static persistence file (default:
		// joynr.properties in working dir) at
		// runtime. If not available in the working dir, it will be created
		// during the first launch
		// of the application. Copy the following lines to the custom
		// persistence file to set a
		// certain joynr server instance.
		// NOTE: This application uses a custom static persistence file
		// provider-joynr.properties.
		// Copy the following lines to the custom persistence file to set a
		// certain joynr server
		// instance.
		// joynr.messaging.bounceproxyurl=http://localhost:8080/bounceproxy/
		// joynr.messaging.capabilitiesdirectoryurl=http://localhost:8080/bounceproxy/channels/discoverydirectory_channelid/
		// joynr.messaging.channelurldirectoryurl=http://localhost:8080/bounceproxy/channels/discoverydirectory_channelid/

		// 3) Or set them in Java System properties.
		// -Djoynr.messaging.bounceProxyUrl=http://localhost:8080/bounceproxy/
		// -Djoynr.messaging.capabilitiesDirectoryUrl=http://localhost:8080/bounceproxy/channels/discoverydirectory_channelid/
		// -Djoynr.messaging.channelUrlDirectoryUrl=http://localhost:8080/bounceproxy/channels/discoverydirectory_channelid/

		// NOTE:
		// Programmatically set configuration properties override properties set
		// in the static persistence file.
		// Java system properties override both

		// Application-specific configuration properties are injected to the
		// application by setting
		// them on the JoynApplicationModule.
		Properties appConfig = new Properties();

		// the following line is required in case of java<->javascript use case,
		// as long as javascript is not using channelurldirectory and
		// globalcapabilitiesdirectory
		Module[] modules = new Module[] {}; // new DefaultUrlDirectoryModule()};
		JoynrApplication joynrApplication = new JoynrInjectorFactory(
				joynrConfig, modules)
				.createApplication(new JoynrApplicationModule(
						MyRadioProviderApplication.class, appConfig));
		joynrApplication.run();

		MyRadioHelper.pressQEnterToContinue();

		joynrApplication.shutdown();
	}

	@Override
	public void run() {
		provider = new MyRadioProvider();
		runtime.registerCapability(localDomain, provider, RadioProvider.class,
				AUTH_TOKEN);
	}

	@Override
	public void shutdown() {
		if (provider != null) {
			runtime.unregisterCapability(localDomain, provider,
					RadioProvider.class, AUTH_TOKEN);
		}
		runtime.shutdown(true);
	}
}
