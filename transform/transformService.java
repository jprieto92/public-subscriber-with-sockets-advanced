package transform;

import javax.jws.WebService;
import javax.jws.WebMethod;
import java.util.Random;

@WebService
public class transformService{
	@WebMethod 
	public String transformText(String texto){
		String devolver = "";
		texto = texto.trim();
		texto = texto.replaceAll(" {2,}", " ");
		String[] aux = texto.split(" ");
		if(aux.length == 0 || ((aux.length == 1) && (aux[0].length() == 0)))
		{
			return texto;
		}
		for(int i = 0; i< aux.length; i++)
		{
			if(i == aux.length-1)
			{
				devolver+=aux[i].substring(0,1).toUpperCase()+aux[i].substring(1);
			}else
			{
				devolver+=aux[i].substring(0,1).toUpperCase()+aux[i].substring(1) + " ";
			}
		}

		return devolver;
	}
}
