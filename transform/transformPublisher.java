package transform;

import javax.xml.ws.Endpoint;

public class transformPublisher{
	public static void main(String[] args){
		final String url = "http://localhost:8000/transform";
		System.out.println("Publishing transform text at endpoint: " + url);
		Endpoint.publish(url,new transformService());
	}	
}
