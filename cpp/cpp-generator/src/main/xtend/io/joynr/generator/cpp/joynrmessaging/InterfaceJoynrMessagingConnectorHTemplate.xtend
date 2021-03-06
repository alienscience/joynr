package io.joynr.generator.cpp.joynrmessaging
/*
 * !!!
 *
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
 *
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
 */

import com.google.inject.Inject
import org.franca.core.franca.FInterface
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions

class InterfaceJoynrMessagingConnectorHTemplate {
	
	@Inject
	private extension TemplateBase
	
	@Inject
	private extension JoynrCppGeneratorExtensions
	
	def generate(FInterface serviceInterface) {
		val interfaceName = serviceInterface.joynrName;
		val headerGuard = ("GENERATED_INTERFACE_"+getPackagePathWithJoynrPrefix(serviceInterface, "_")+"_"+interfaceName+"JoynrMessagingConnector_h").toUpperCase
		'''
		«warning()»
		
		#ifndef «headerGuard»
		#define «headerGuard»
		
		«getDllExportIncludeStatement()»
		#include "«getPackagePathWithJoynrPrefix(serviceInterface, "/")»/I«interfaceName»Connector.h"
		#include "joynr/AbstractJoynrMessagingConnector.h"
		#include "joynr/JoynrMessagingConnectorFactory.h"
		
		namespace joynr {
			class MessagingQos;
			class IJoynrMessageSender;
			class SubscriptionManager;
		}
		
		«getNamespaceStarter(serviceInterface)»
		
		
		class «getDllExportMacro()» «interfaceName»JoynrMessagingConnector : public I«interfaceName»Connector, virtual public joynr::AbstractJoynrMessagingConnector {
		public:
		    «interfaceName»JoynrMessagingConnector(
		        joynr::IJoynrMessageSender* messageSender,
		        joynr::SubscriptionManager* subscriptionManager,
		        const QString &domain,
		        const QString proxyParticipantId,
		        const QString& providerParticipantId,
		        const joynr::MessagingQos &qosSettings,
		        joynr::IClientCache *cache,
		        bool cached,
		        qint64 reqCacheDataFreshness_ms);
		
		
			virtual bool usesClusterController() const;
			
			virtual ~«interfaceName»JoynrMessagingConnector(){}
		
			«FOR attribute: getAttributes(serviceInterface)»
				«val returnType = getMappedDatatypeOrList(attribute)»
				«val attributeName = attribute.joynrName»
				virtual void get«attributeName.toFirstUpper»(joynr::RequestStatus& status, «getMappedDatatypeOrList(attribute)»& «attributeName»);
				virtual void get«attributeName.toFirstUpper»(QSharedPointer<joynr::Future<«getMappedDatatypeOrList(attribute)»> > future, QSharedPointer< joynr::ICallback<«getMappedDatatypeOrList(attribute)»> > callBack);
				virtual void get«attributeName.toFirstUpper»(QSharedPointer<joynr::Future<«getMappedDatatypeOrList(attribute)»> > future);
				virtual void get«attributeName.toFirstUpper»(QSharedPointer<joynr::ICallback<«getMappedDatatypeOrList(attribute)»> > callBack);

				virtual void set«attributeName.toFirstUpper»(QSharedPointer<joynr::ICallback<void> > callBack, «getMappedDatatypeOrList(attribute)» «attributeName»);
				virtual void set«attributeName.toFirstUpper»(joynr::RequestStatus &status, const «getMappedDatatypeOrList(attribute)»& «attributeName»);
				virtual void set«attributeName.toFirstUpper»(QSharedPointer<joynr::Future<void> > future, QSharedPointer< joynr::ICallback<void> > callBack, «getMappedDatatypeOrList(attribute)» «attributeName»);
				virtual void set«attributeName.toFirstUpper»(QSharedPointer<joynr::Future<void> > future, «getMappedDatatypeOrList(attribute)» «attributeName»);

				virtual QString subscribeTo«attributeName.toFirstUpper»(QSharedPointer<joynr::ISubscriptionListener<«returnType»> > subscriptionListener, QSharedPointer<joynr::SubscriptionQos> subscriptionQos);
				virtual void unsubscribeFrom«attributeName.toFirstUpper»(QString& subscriptionId);
			«ENDFOR»
		    
			«FOR method: getMethods(serviceInterface)»
				«val methodName = method.joynrName»
				«IF getMappedOutputParameter(method).head == "void"»
					virtual void «methodName» (joynr::RequestStatus &status «prependCommaIfNotEmpty(getCommaSeperatedTypedParameterList(method))»);
				«ELSE»
					virtual void «methodName» (joynr::RequestStatus &status«prependCommaIfNotEmpty(getCommaSeperatedTypedOutputParameterList(method))»«prependCommaIfNotEmpty(getCommaSeperatedTypedParameterList(method))»);
				«ENDIF»
				virtual void «methodName» (QSharedPointer<joynr::Future<«getMappedOutputParameter(method).head»> > future, QSharedPointer< joynr::ICallback<«getMappedOutputParameter(method).head» > > callBack «prependCommaIfNotEmpty(getCommaSeperatedTypedParameterList(method))»);
				virtual void «methodName» (QSharedPointer<joynr::Future<«getMappedOutputParameter(method).head»> > future «prependCommaIfNotEmpty(getCommaSeperatedTypedParameterList(method))»);
				virtual void «methodName» (QSharedPointer<joynr::ICallback<«getMappedOutputParameter(method).head»> > callBack «prependCommaIfNotEmpty(getCommaSeperatedTypedParameterList(method))»);
			«ENDFOR»
		};
		«getNamespaceEnder(serviceInterface)»

		namespace joynr {
			
		// Helper class for use by the JoynrMessagingConnectorFactory
		// This class creates instances of «interfaceName»JoynrMessagingConnector
		template <>
		class JoynrMessagingConnectorFactoryHelper <«getPackagePathWithJoynrPrefix(serviceInterface, "::")»::I«interfaceName»Connector> {
		public:
		    «getPackagePathWithJoynrPrefix(serviceInterface, "::")»::«interfaceName»JoynrMessagingConnector* create(
		            joynr::IJoynrMessageSender* messageSender,
		            joynr::SubscriptionManager* subscriptionManager,
		            const QString &domain,
		            const QString proxyParticipantId,
		            const QString& providerParticipantId,
		            const joynr::MessagingQos &qosSettings,
		            joynr::IClientCache *cache,
		            bool cached,
		            qint64 reqCacheDataFreshness_ms
		    ) {
		        return new «getPackagePathWithJoynrPrefix(serviceInterface, "::")»::«interfaceName»JoynrMessagingConnector(
		        		messageSender,
		        		subscriptionManager,
		        		domain,
		        		proxyParticipantId,
		        		providerParticipantId,
		        		qosSettings,
		        		cache,
		        		cached,
		        		reqCacheDataFreshness_ms
		        );
		    }
		};
		
		} // namespace joynr
		#endif // «headerGuard»
		'''
	}
}