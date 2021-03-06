package io.joynr.generator.cpp.util
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


class InterfaceSubscriptionUtil {
	@Inject	extension JoynrCppGeneratorExtensions

	def produceSubscribeUnsubscribeMethods(FInterface serviceInterface, boolean pure)
	'''
		«FOR attribute: getAttributes(serviceInterface)»
			«val returnType = getMappedDatatypeOrList(attribute)»
			virtual QString subscribeTo«attribute.joynrName.toFirstUpper»(QSharedPointer<joynr::ISubscriptionListener<«returnType»> > subscriptionListener, QSharedPointer<joynr::SubscriptionQos> subscriptionQos)«IF pure» = 0«ENDIF»;
			virtual void unsubscribeFrom«attribute.joynrName.toFirstUpper»(QString& subscriptionId)«IF pure» = 0«ENDIF»;
	    «ENDFOR»
	'''
}