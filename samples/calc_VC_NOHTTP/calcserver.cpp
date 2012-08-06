#include "soapH.h"
#include "calc.nsmap"


#ifdef WITH_NOHTTP
int main(int argc, char **argv)
{
   struct soap soap; 
   int m, s; // master and slave sockets 
   soap_init(&soap); 
   // m = soap_bind(&soap, "machine.cs.fsu.edu", 18083, 100); 
   // ESL changed to:
   m = soap_bind(&soap, "192.168.0.3", 9001, 100); 
   if (m < 0) 
      soap_print_fault(&soap, stderr); 
   else
   { 
      fprintf(stderr, "Socket connection successful: master socket = %d\n", m); 
      for (int i = 1; ; i++) 
      {
         s = soap_accept(&soap); 
         if (s < 0) 
         { 
            soap_print_fault(&soap, stderr); 
            break; 
         } 
         fprintf(stderr, "%d: accepted connection from IP=%d.%d.%d.%d socket=%d", i, 
            (soap.ip >> 24)&0xFF, (soap.ip >> 16)&0xFF, (soap.ip >> 8)&0xFF, soap.ip&0xFF, s); 
         if (soap_serve(&soap) != SOAP_OK) // process RPC request 
            soap_print_fault(&soap, stderr); // print error 
         fprintf(stderr, "request served\n"); 
         soap_destroy(&soap); // clean up class instances 
         soap_end(&soap); // clean up everything and close socket 
      } 
   } 
   soap_done(&soap); // close master socket and detach environment 
} 

#else 
 
int main(int argc, char **argv)
{ int m, s; /* master and slave sockets */
  struct soap soap;
  soap_init(&soap);
  if (argc < 2)
    soap_serve(&soap);	/* serve as CGI application */
  else
  { m = soap_bind(&soap, NULL, atoi(argv[1]), 100);
    if (m < 0)
    { soap_print_fault(&soap, stderr);
      exit(-1);
    }
    fprintf(stderr, "Socket connection successful: master socket = %d\n", m);
    for ( ; ; )
    { s = soap_accept(&soap);
      fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
      if (s < 0)
      { soap_print_fault(&soap, stderr);
        exit(-1);
      } 
      soap_serve(&soap);
      soap_end(&soap);
    }
  }
  return 0;
} 
#endif

int ns__add(struct soap *soap, double a, double b, double *result)
{ *result = a + b;
  return SOAP_OK;
} 

int ns__sub(struct soap *soap, double a, double b, double *result)
{ *result = a - b;
  return SOAP_OK;
} 

int ns__mul(struct soap *soap, double a, double b, double *result)
{ *result = a * b;
  return SOAP_OK;
} 

int ns__div(struct soap *soap, double a, double b, double *result)
{ if (b)
    *result = a / b;
  else
  { char *s = (char*)soap_malloc(soap, 1024);
    sprintf(s, "<error xmlns=\"http://tempuri.org/\">Can't divide %f by %f</error>", a, b);
    return soap_sender_fault(soap, "Division by zero", s);
  }
  return SOAP_OK;
} 

int ns__pow(struct soap *soap, double a, double b, double *result)
{ *result = pow(a, b);
  if (soap_errno == EDOM)	/* soap_errno is like errno, but compatible with Win32 */
  { char *s = (char*)soap_malloc(soap, 1024);
    sprintf(s, "Can't take the power of %f to %f", a, b);
    sprintf(s, "<error xmlns=\"http://tempuri.org/\">Can't take power of %f to %f</error>", a, b);
    return soap_sender_fault(soap, "Power function domain error", s);
  }
  return SOAP_OK;
} 
