/* 	JAVIER PRIETO CEPEDA		100307011 
	PEDRO ROBLEDO QUINTERO	 	100XXXXXX
*/


import java.io.*;
import gnu.getopt.Getopt;
import java.lang.* ;
import java.net.* ;
import java.util.* ;
import java.nio.*;
import client.TransformServiceService;
import client.TransformService;
import java.util.List;


class suscriptor {
	
	/*HILO SERVIDOR EN EL SUSCRIPTOR*/
	public static class MyThread implements Runnable {
		Socket sc = null;
		ServerSocket serverSc = null;

		/*CONSTRUCTOR DEL THREAD*/
		public MyThread(ServerSocket serverSc)
		{
			this.serverSc = serverSc;
		}

		/*FUNCION QUE EJECUTA EL THREAD*/
		public void run()
		{
			DataInputStream bytesLeidos;
			String message;
			String [] parseMessage;
			/*ESPERAMOS CONEXION*/
			while(true)
			{
				try
				{
					sc = serverSc.accept();
					//System.out.println("ACEPTO"); // prueba
					bytesLeidos = new DataInputStream(sc.getInputStream());
					message = readLine(bytesLeidos, MAX_MESSAGE);
					if(message.equals("KILL"))
					{
						sc.close();
						break;
					}
					sc.close();
					//System.out.println("MESSAGE: "+ message); // prueba
					/******/					
					/*SPLIT DEL MENSAJE, Y PONER EL FORMATO*/
					/******/
					parseMessage = message.split(":", 2);
					/*WEB SERVICE*/
					URL url = new URL(_urlServicioWeb);
					TransformServiceService service = new TransformServiceService(url);
					TransformService port = service.getTransformServicePort();
					// llamadas
					String res = "";
					res = port.transformText(parseMessage[1]);		
					/*FIN WEB SERVICE*/

					System.out.println("MESSAGE FROM " + parseMessage[0] + " : " + res); // prueba
					System.out.print("c> ");
				}catch(Exception e){
					/*CUALQUIER EXCEPCION A TRATAR*/
					System.err.println("Problema"); // prueba
					e.printStackTrace() ;
                                }
			}

		}

	}


	/********************* TYPES **********************/
	
	
	/******************* ATTRIBUTES *******************/
	
	private static Thread myThread;		// Hilo 
	private static String ip = null;	// Almacenar Ip
	private static short port = 1023;	// Almacenar puerto
	
	private static int MAX_MESSAGE = 1154;
	private static int IP_SIZE = 16;
	private static int MAX_TOPIC = 128;
	private static int MAX_TEXT = 1024;

	private static String _server   = null;
	private static String _urlServicioWeb = null;
	private static short _port = -1;
	private static int MAX_SIZE = 256;
	private static String USUARIO = null;

	private static ArrayList<String> listSuscriptions = new ArrayList<String>();
		
	
	/********************* METHODS ********************/
	
	/*LEER CADENAS DE CARACTERES*/
	static String readLine(DataInputStream istream, int n) throws Exception
	{
	        int totRead;		// Maximo a leer
	        String buf;
	        char ch;

	        if (n <= 0)
		        return ""; 
	     
	        buf = "";
	        totRead = 0;
	 
	        while (true) {
                	ch = (char) istream.readByte();	// Leer un byte, un caracter			
			// System.out.println("buffer: "+ ch); // prueba
                    	if (ch == '\0')
                        	break;
                    	if (totRead < n - 1) {		// Descartamos el resto
			        totRead++;
			        buf += ch; 
			}
		         
	        }
	        return buf;
        }     

	/*OBTENER DIRECCION IP DE LOCALHOST*/
	public static String getLocalAddress()
	{
		String my_ip = null;
  		try 
		{
        		Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
        		while (interfaces.hasMoreElements()) 
			{
           		        NetworkInterface iface = interfaces.nextElement();
            		        // filters out 127.0.0.1 and inactive interfaces
            		        if (iface.isLoopback() || !iface.isUp())
                		continue;

            		        Enumeration<InetAddress> addresses = iface.getInetAddresses();
            		        while(addresses.hasMoreElements()) 
				{
                	        	InetAddress addr = addresses.nextElement();
                	        	my_ip = addr.getHostAddress();
                	        	//System.err.println(iface.getDisplayName() + " " + my_ip);
            		        }
        	        }
    		} catch (SocketException e) {
        		throw new RuntimeException(e);
    		}           
            	return my_ip;
	}

	/*FINALIZAR HILO SERVIDOR*/
	static int killThread()
	{
	        Socket sd;
	        DataOutputStream ostream;
		try {
		        /*CREO LA CONEXION*/
		   	sd = new Socket(ip, port);
		        /*CREAMOS EL FLUJO DEL ENVIO*/
			ostream = new DataOutputStream(sd.getOutputStream());
			ostream.writeBytes("KILL\0");

			sd.close();
			
			myThread.join();
		}catch(Exception e){
			return 1;
		}
			
		port = 1024;
		ip = null;
			
		return 0;
	}

	static int subscribe(String topic) 
	{
		// Write your code here
		/*LOS TEMAS, COMO MAXIMO, 128 BYTES*/
		if(topic.length() > 128){
			topic = topic.substring(0,128);		
		}
		
                Socket sc;	// Socket que sirve para 
		DataOutputStream ostream;
		DataInputStream istream; 
		String operation="";
		byte result;	

		InetAddress direcc = null;
 		try
 		{
 			direcc = InetAddress.getByName(_server);
 		}
 		catch(UnknownHostException uhe)
 		{
 			System.err.println("Host no encontrado : " + uhe);
 			return 1;
 		} 
		
		//System.out.println("IP: " +  ip);
		//System.out.println("Puerto: "+ port);
		//System.out.println("listSuscriptions.size(): " + listSuscriptions.size()); // prueba
		if(listSuscriptions.contains(topic) == false && listSuscriptions.size() == 0)
		{
			listSuscriptions.add(topic);
			/*CREAMOS HILO SERVIDOR QUE ATIENDE A LOS ENVIOS DE TEXTO SI HAY SUSCRIPCIONES*/
			ServerSocket serverAddr = null;
		
			/*ALMACENAMOS IP Y PUERTO DEL SUSCRIPTOR PARA ESCUCHAR*/
			while(serverAddr == null)
			{
				try
				{
					ip = getLocalAddress();
					serverAddr = new ServerSocket(port);
				}
				catch(Exception e){
					port++;
				}
			}	
			myThread = new Thread(new MyThread(serverAddr));      // Hilo escuchador del tema nuevo
			myThread.start();
		
		}
		if(listSuscriptions.contains(topic) == false)
		{
			listSuscriptions.add(topic);
		}		
		//System.out.println("listSuscriptions.size(): " + listSuscriptions.size()); // prueba
		//System.out.println("Puerto:" + port);				
		try{
		        /*CREAMOS LA CONEXION*/
		        sc = new Socket(direcc, _port);
		}catch (Exception e){
			//e.printStackTrace() ;	// Error en el metodo
			System.out.println("Error in the connection to the broker "+ _server + ":" +_port);
			listSuscriptions.remove(topic);
			if(listSuscriptions.size() == 0)
				return killThread(); // Matar hilo si no hay suscripciones restantes
			return 1;
		}
		try{
		        /*CREAMOS EL FLUJO PARA EL ENVIO AL BROKER*/
			ostream = new DataOutputStream(sc.getOutputStream());
			
			/*CREAMOS FLUJO DE RECEPCION DEL BROKER*/
			istream = new DataInputStream(sc.getInputStream());   
			    
			/*PETICION SUBSCRIBE*/
			ostream.writeBytes("SUBSCRIBE\0");	// Operacion
			ostream.flush();
			ostream.writeBytes(topic + "\0");	// Tema
			ostream.flush();
			ostream.writeShort(port);		// Puerto Escuchador
			ostream.flush();
		        /*RECIBIMOS LA RESPUESTA*/
			result = istream.readByte();
	                
	                /*FINALIZAMOS CONEXION*/
	                sc.close();
	                
	                /*SUSCRIPCION CORRECTA*/
	                if(result == 0)
			{
				System.out.println("c> SUBSCRIBE OK");
	                        return 0;
	                }
	                /*SUSCRIPCION INCORRECTA*/
	                if(result == 1){
				System.out.println("c> SUBSCRIBE FAIL");
				listSuscriptions.remove(topic);
				if(listSuscriptions.size() == 0)
					return killThread(); // Matar hilo si no hay suscripciones restantes
		
			}
		}
		catch (Exception e){
			//e.printStackTrace() ;	// Error en el metodo
			System.out.println("c> NETWORK ERROR");			
			listSuscriptions.remove(topic);
			if(listSuscriptions.size() == 0)
				return killThread(); // Matar hilo si no hay suscripciones restantes			
			return 1;
		} 	
		return 0;
	}

	static int unsubscribe(String topic) 
	{
		// Write your code here
		if(topic.length() > 128){
			topic = topic.substring(0,128);		
		}
		
		InetAddress direcc = null;
 		try
 		{
 			direcc = InetAddress.getByName(_server);
 		}
 		catch(UnknownHostException uhe)
 		{
 			System.err.println("Host no encontrado : " + uhe);
 			return 1;
 		} 

                Socket sc;	// Socket que sirve para 
		DataOutputStream ostream;
		DataInputStream istream; 
		String operation="";
		byte result;
		try{
		        /*CREAMOS LA CONEXION*/
		        sc = new Socket(direcc, _port);
		}catch (Exception e){
			//e.printStackTrace() ;	// Error en el metodo
			System.out.println("Error in the connection to the broker "+ _server + ":" +_port);
			listSuscriptions.remove(topic);
			if(listSuscriptions.size() == 0)
				return killThread(); // Matar hilo si no hay suscripciones restantes
			return 1;
		} 	
		try{       
		        /*CREAMOS EL FLUJO PARA EL ENVIO AL BROKER*/
			ostream = new DataOutputStream(sc.getOutputStream());
			
			/*CREAMOS FLUJO DE RECEPCION DEL BROKER*/
			istream = new DataInputStream(sc.getInputStream());   
			    
			/*PETICION UNSUBSCRIBE*/
			ostream.writeBytes("UNSUBSCRIBE\0");	// Operacion
			ostream.flush();
			ostream.writeBytes(topic + "\0");	// Tema
			ostream.flush();
			ostream.writeShort(port);		// Puerto Escuchador
			ostream.flush();
		        /*RECIBIMOS LA RESPUESTA*/
			result = istream.readByte();
	                
	                /*FINALIZAMOS LA CONEXION*/
	                sc.close();
	                
	                /*QUITAR SUSCRIPCION CORRECTO*/
	                if(result == 0)
			{
				System.out.println("c> UNSUBSCRIBED OK");
				listSuscriptions.remove(topic);
				if(listSuscriptions.size() == 0)
					return killThread(); // Matar hilo si no hay suscripciones restantes
	                        return 0;
	                }
	                /*NO ESTABA SUSCRITO*/
	                if(result == 1){
				System.out.println("c> TOPIC NOT SUBSCRIBED");
				return 0;
			}			
	                /*NO ESTABA SUSCRITO*/
	                if(result == 2){
				System.out.println("c> UNSUBSCRIBED FAIL");
				return 0;
			}
		}
		catch (Exception e){
			//e.printStackTrace() ;	// Error en el metodo
			System.out.println("c> NETWORK ERROR");
			return 1;
		} 	
		return 0;
	}
	
	
	
	/**
	 * @brief Command interpreter for the suscriptor. It calls the protocol functions.
	 */
	static void shell() 
	{
		boolean exit = false;
		String input;
		String [] line;
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		while (!exit) {
			try {
				System.out.print("c> ");
				input = in.readLine();
				line = input.split("\\s");

				if (line.length > 0) {
					/*********** SUBSCRIBE *************/
					if (line[0].equals("SUBSCRIBE")) {
						if  (line.length == 2) {
							subscribe(line[1]); // topic = line[1]
						} else {
							System.out.println("Syntax error. Usage: SUBSCRIBE <topic>");
						}
					} 
					
					/********** UNSUBSCRIBE ************/
					else if (line[0].equals("UNSUBSCRIBE")) {
						if  (line.length == 2) {
							unsubscribe(line[1]); // topic = line[1]
						} else {
							System.out.println("Syntax error. Usage: UNSUBSCRIBE <topic>");
						}
                    } 
                    
                    /************** QUIT **************/
                    else if (line[0].equals("QUIT")){
						if (line.length == 1) {
							int numTopics=listSuscriptions.size();
							while(numTopics>0)
							{
								unsubscribe(listSuscriptions.get(0));
								numTopics--;
							}
							exit = true;
						} else {
							System.out.println("Syntax error. Use: QUIT");
						}
					} 
					
					/************* UNKNOWN ************/
					else {						
						System.out.println("Error: command '" + line[0] + "' not valid.");
					}
				}				
			} catch (java.io.IOException e) {
				System.out.println("Exception: " + e);
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * @brief Prints program usage
	 */
	static void usage() 
	{
		System.out.println("Usage: java -cp . suscriptor -s <server> -p <port>");
	}
	
	/**
	 * @brief Parses program execution arguments 
	 */ 
	static boolean parseArguments(String [] argv) 
	{
		Getopt g = new Getopt("suscriptor", argv, "ds:p:u:");

		int c;
		String arg;

		while ((c = g.getopt()) != -1) {
			switch(c) {
				//case 'd':
				//	_debug = true;
				//	break;
				case 's':
					_server = g.getOptarg();
					break;
				case 'p':
					arg = g.getOptarg();
					_port = Short.parseShort(arg);
					break;
				case 'u':
					_urlServicioWeb = g.getOptarg();
					break;
				case '?':
					System.out.print("getopt() returned " + c + "\n");
					break; // getopt() already printed an error
				default:
					System.out.print("getopt() returned " + c + "\n");
			}
		}
		
		if (_server == null)
			return false;
		
		if ((_port < 1024) || (_port > 65535)) {
			System.out.println("Error: Port must be in the range 1024 <= port <= 65535");
			return false;
		}
		if (_urlServicioWeb == null)
		{
			System.out.println("Error: No url web service inserted");
			return false;
		}

		return true;
	}
	
	
	
	/********************* MAIN **********************/
	
	public static void main(String[] argv) 
	{
		if(!parseArguments(argv)) {
			usage();
			return;
		}
		

		// Write code here
		shell();
		System.out.println("+++ FINISHED +++");
	}
}
