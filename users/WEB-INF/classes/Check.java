import java.util.*;
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import org.json.*;
import java.lang.*;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

public class Check extends HttpServlet
{
	static Logger logger = Logger.getLogger(Check.class.getName());
	public native boolean checks(String serviceName);
	public native boolean stops(String serviceName);
	static
	{
		System.load("C:\\Program Files\\Apache Software Foundation\\Tomcat 8.5\\webapps\\users\\WEB-INF\\lib\\Startingservices.dll");
	}
	public void doGet(HttpServletRequest req,HttpServletResponse res)throws ServletException,IOException
	{
		PrintWriter out = res.getWriter();
		String serviceName = req.getParameter("services");
		String process = req.getParameter("process");
		JSONObject jo = new JSONObject();
		Check c = new Check();
		System.out.println(serviceName);
		System.out.println(process);
		if(process.equals("start"))
		{
			if(c.checks(serviceName))
			{
				System.out.println("In Java Service started SuccessFully");
				logger.log(Level.INFO, "In Java Service started SuccessFully");
				jo.put("result","Service started Successfully");
			}
			else
			{
				System.out.println("In Java Service not started SuccessFully");
				logger.log(Level.INFO, "In Java Service not started SuccessFully");
				jo.put("result","Service not started Successfully");
			}
		}
		else if(process.equals("stop"))
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
