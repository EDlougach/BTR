#ifndef SEND_MAIL_H

#define SEND_MAIL_H

//** SendMail is a C++ wrapper for the Blat email utility.
//**
//** The official website for Blat is at http://www.blat.net/
//**
//** Blat is a public-domain tool for sending emamil using SMTP via command line arguments.
//**
//** Blat is typically used by launching the executable 'blat.exe'.
//** Another option is to use the Blat.dll which exports a single function to invoke Blat.
//**
//** This wrapper program has *EVERYTHING YOU NEED* to send email.
//** Simply include 'SendMail.h' and 'SendMail.cpp' in your project.
//**
//** The BLAT.DLL file is actually embedded directly into the source code of 'SendMail.cpp'!
//**
//** The BLAT.dll is loaded directly for memory using the MemoryModule code written by Joachim Bauch.
//**
//** If you want access to the complete functionality of the blat tool then you can use the 'sendBlat'
//** function and pass all of the command line arguments manually.  Otherwise use the convenience routine
//** 'sendMail' to send an email quickly.
//**
//** This wrapper was written by John W. Ratcliff ( http://codesuppository.blogspot.com ) and released
//** into the public domain on June 21, 2008
//**
//** Under no circumstances should this tool be used to send bulk/spam email!!!
//**
//** My own personal use-case for this code is to have a server send email-alerts to me when it gets in a bad state.
//**
//** *NOTE* This version internally makes use of the MemoryModule routine written by Joachim Bauch and is subject to
//**        the Mozilla license.  Please be aware of that fact if you use this code.

bool sendBlat(int argc,const char *argv[]); // refer to the Blat documentation for the full set of command line arguments.

bool sendMail(const char *mail_server,   // mail server to use for sending the email message.
              const char *from,          // email address of the sender
              const char *to,            // email address to send to.
              const char *subject,       // the subject line of the email.
              const char *body,          // the body text of the email.
              const char *attachment);   // an optional attachment file.

#endif
