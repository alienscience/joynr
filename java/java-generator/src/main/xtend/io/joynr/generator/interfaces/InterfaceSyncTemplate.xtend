package io.joynr.generator.interfaces
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
import io.joynr.generator.util.TemplateBase
import org.franca.core.franca.FInterface
import io.joynr.generator.util.JoynrJavaGeneratorExtensions

class InterfaceSyncTemplate {
  @Inject extension JoynrJavaGeneratorExtensions
  @Inject extension TemplateBase

	def generate(FInterface serviceInterface) {
		val interfaceName =  serviceInterface.joynrName
		val syncClassName = interfaceName + "Sync"
		val packagePath = getPackagePathWithJoynrPrefix(serviceInterface, ".")
			
'''
«warning()»
		
package «packagePath»; 
         
import java.util.List;
		
import com.fasterxml.jackson.core.type.TypeReference;

import io.joynr.dispatcher.rpc.JoynrSyncInterface;
import io.joynr.dispatcher.rpc.annotation.JoynrRpcReturn;		
import io.joynr.dispatcher.rpc.annotation.JoynrRpcParam;

import io.joynr.exceptions.JoynrArbitrationException;

«FOR datatype: getRequiredIncludesFor(serviceInterface)»
	import «datatype»;
«ENDFOR»
//TODO: Only include the necessary imports in the xtend template. This needs to be checked depending on the fibex. 
@SuppressWarnings("unused")

public interface «syncClassName» extends «interfaceName», JoynrSyncInterface {

«FOR attribute: getAttributes(serviceInterface)»
	«var attributeName = attribute.joynrName»
	«var attributeType = getObjectDataTypeForPlainType(getMappedDatatypeOrList(attribute))» 
	«var getAttribute = "get" + attributeName.toFirstUpper»
	«var setAttribute = "set" + attributeName.toFirstUpper»

		«IF isReadable(attribute)»
		@JoynrRpcReturn(deserialisationType = «getTokenTypeForArrayType(attributeType)»Token.class)
		public «attributeType» «getAttribute»() throws JoynrArbitrationException;
		«ENDIF»		
		«IF isWritable(attribute)»			
			void «setAttribute»(@JoynrRpcParam(value="«attributeName»", deserialisationType = «getTokenTypeForArrayType(attributeType)»Token.class) «attributeType» «attributeName») throws JoynrArbitrationException;		
		«ENDIF»			
«ENDFOR»


«FOR method: getMethods(serviceInterface)»
	«var methodName = method.joynrName»
	«var params = getTypedParameterListJavaRpc(method)»
		/*
		* «methodName»
		*/
		«IF getMappedOutputParameter(method).iterator.next=="void"»
		public void «methodName»(«params») throws JoynrArbitrationException;
		«ELSE»
		@JoynrRpcReturn(deserialisationType = «getTokenTypeForArrayType(getMappedOutputParameter(method).iterator.next)»Token.class)
		public «getObjectDataTypeForPlainType(getMappedOutputParameter(method).iterator.next)» «methodName»(«params») throws JoynrArbitrationException;
	«ENDIF»
«ENDFOR»
}

'''
	}	
}