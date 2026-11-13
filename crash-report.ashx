<%@ WebHandler Language="C#" Class="PristonCrashReport" %>
// crash-report.ashx - the game sends a crash report here as it happens (the test page's
// AddressSanitizer reports: web\ptwin\src\ptwin_kernel.cpp), so it reaches the server even
// when the page freezes right after. Saved to App_Data\crash-reports\ (IIS never serves
// App_Data): one text file per report, with the time and the browser. POST only, at most
// 512 KB a report and 500 reports kept (then refused until some are deleted).
using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Web;

public class PristonCrashReport : IHttpHandler
{
	const int MaxBytes = 512 * 1024;
	const int MaxFiles = 500;

	public bool IsReusable { get { return true; } }

	public void ProcessRequest( HttpContext context )
	{
		HttpRequest req = context.Request;
		HttpResponse res = context.Response;
		if ( req.HttpMethod != "POST" ) { res.StatusCode = 405; return; }
		int len = req.ContentLength;
		if ( len <= 0 || len > MaxBytes ) { res.StatusCode = 413; return; }

		string dir = context.Server.MapPath( "~/App_Data/crash-reports" );
		Directory.CreateDirectory( dir );
		if ( Directory.GetFiles( dir ).Length >= MaxFiles ) { res.StatusCode = 507; return; }

		byte[] body = new byte[len];
		int got = 0, n;
		while ( got < len && ( n = req.InputStream.Read( body, got, len - got ) ) > 0 ) got += n;

		string kind = Regex.Replace( req.QueryString["kind"] ?? "report", "[^a-zA-Z0-9-]", "" );
		if ( kind.Length == 0 ) kind = "report";
		if ( kind.Length > 24 ) kind = kind.Substring( 0, 24 );
		string name = DateTime.Now.ToString( "yyyyMMdd-HHmmss-fff" ) + "-" + kind + "-" +
			Guid.NewGuid().ToString( "N" ).Substring( 0, 6 ) + ".txt";
		string head = "time " + DateTime.Now.ToString( "yyyy-MM-dd HH:mm:ss" ) + "\r\nbrowser " + ( req.UserAgent ?? "?" ) + "\r\n\r\n";

		using ( FileStream fs = new FileStream( Path.Combine( dir, name ), FileMode.CreateNew ) ) {
			byte[] h = Encoding.UTF8.GetBytes( head );
			fs.Write( h, 0, h.Length );
			fs.Write( body, 0, got );
		}
		res.StatusCode = 204;
	}
}
