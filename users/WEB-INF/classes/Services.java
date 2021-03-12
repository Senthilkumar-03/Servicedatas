import java.util.*;
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import org.json.*;
import java.lang.*;

public class Services extends HttpServlet
{
	public native ArrayList services();        // JNI method which returns arraylist from C++
	static
	{
		System.load("C:\\Program Files\\Apache Software Foundation\\Tomcat 8.5\\webapps\\users\\WEB-INF\\lib\\service.dll");
	}
	
	// doGet method is hitted by the request from the frontend and executes
	
	public void doGet(HttpServletRequest req,HttpServletResponse res)throws ServletException,IOException
	{
		PrintWriter out = res.getWriter();               
		ArrayList<HashMap> al = new ArrayList<>();
		Services s = new Services();
		al = s.services();
		int len = al.size();
		JSONObject jo = new JSONObject();
		jo.put("servicedata",al);                 // Adding the arraylist as value with key "servicedata"
		out.print(jo);
		
	 } 
	
}
	
	
