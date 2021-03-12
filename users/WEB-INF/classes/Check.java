import java.util.*;
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import org.json.*;
import java.lang.*;

public class Check extends HttpServlet
{
	public native boolean checks(String serviceName);   // JNI method for to check and start the services.
	public native boolean stops(String serviceName);    // JNI method for to check and stop the services.
	static
	{
		System.load("C:\\Program Files\\Apache Software Foundation\\Tomcat 8.5\\webapps\\users\\WEB-INF\\lib\\Startingservices.dll");
	}
	
	// doGet method is hitted by the request from the frontend and accessed.
	
	public void doGet(HttpServletRequest req,HttpServletResponse res)throws ServletException,IOException
	{
		PrintWriter out = res.getWriter();
		String serviceName = req.getParameter("services");    // request with servicename from frontend
		String process = req.getParameter("process");         // request with whattype of process from the frontend
		JSONObject jo = new JSONObject();
		Check c = new Check();
		//System.out.println(serviceName);
		//System.out.println(process);
		if(process.equals("start"))                        // if the process as start the function calls the checks method
		{
			if(c.checks(serviceName))
			{
				//System.out.println("In Java Service started SuccessFully");
				jo.put("result","Service started Successfully");
			}
			else                      
			{
				//logger.log(Level.INFO, "In Java Service not started SuccessFully");
				jo.put("result","Service not started Successfully");
			}
		}
		else if(process.equals("stop"))                   // else the process as stop the function calls the stopsmethod
		{
			if(c.stops(serviceName))
			{
				System.out.println("In Java Service stopped SuccessFully");
				jo.put("result","Service stopped Successfully");
			}
			else
			{
				System.out.println("In Java Service not stopped SuccessFully");
				jo.put("result","Service not stopped Successfully");
			}
		}
		out.print(jo);

	}
}
