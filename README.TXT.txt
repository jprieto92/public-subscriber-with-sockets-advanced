Javier Prieto Cepeda
Pedro Robledo Quintero

Para la compilación, se han de seguir los siguientes pasos (indicados también en la memoria):

/*RECOMPILAR PARA QUE FUNCIONE EN GUERNIKA*/

make clean
javac transform/transformService.java 
javac transform/transformPublisher.java
wsimport -p client -keep TransformServiceService.wsdl

/*FIN RECOMPILAR*/

rpcgen -a -N -M almacenamientoTemas.x
make -f Makefile.almacenamientoTemas 
javac transform/transformService.java
javac transform/transformPublisher.java 
make

En caso de hacer make clean, es necesario repetir los pasos anteriores.

Para la ejecución, se han de seguir los siguientes pasos (indicados también en la memoria):

# Servidor de textos 
./serverAlmacenamiento 

#broker 
./broker <puertoE> <puertoS> <ipServerAlmacenamiento> 

#publisher 
./publisher <tema> <ipBroker> <puertoBroker>

#servicioWeb 
java transform.transformPublisher 

#suscriptor 
java -cp . suscriptor -s <ipBroker> -p <puertoBrokerSusc> -u <urlWebService>
