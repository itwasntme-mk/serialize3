///\file external_app_launcher.cpp

#include "external_app_launcher.h"

#if defined(_WIN32)
#include <process.h> // For spawnv
#include <windows.h> // For SetConsoleCtrlHanlder
#elif defined(UNIX) || defined(__linux__)
#include <boost/tokenizer.hpp>

#include <cstdlib>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <cstring>
#include <memory>

namespace
{

#if defined(_WIN32)
std::string FetchLastErrorMessage()
  {
  DWORD lastError = GetLastError();
  if (lastError != 0)
    {
    LPVOID lpMsgBuf;
    FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,  
                    lastError, 
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL );

    std::string errorString(static_cast<const char*>(lpMsgBuf));
    LocalFree( static_cast<char*>(lpMsgBuf) );
    return errorString;
    }
  else
    return "";
  }
#endif // #if defined(_WIN32)

} // namespace

bool LaunchExternalConsoleProcess(const char* commandLine, const char* stdOutFileName,
  std::string& errorString)
  {
  if (commandLine == NULL || stdOutFileName == NULL)
    return false;
  
#if defined(UNIX) || defined(__linux__)
  /*std::string cmdLine(commandLine);
  cmdLine += " &> ";
  cmdLine += stdOutFileName;
  int result = std::system("cmdLine.c_str()");
  return result;*/

  int stdOutFileDescriptor = open(stdOutFileName, O_WRONLY|O_CREAT|O_APPEND,0644);
  if (stdOutFileDescriptor == -1)
    return false;

  // parse the command line into an argv array
  std::vector<std::string> commandLineTokens;
  std::string commandLineString(commandLine);
  std::replace(commandLineString.begin(), commandLineString.end(), '\\', '/');
  
  boost::tokenizer<boost::escaped_list_separator<char> > 
    commandLineTokenizer(commandLineString, boost::escaped_list_separator<char>("\\", " \t", "'\""));

  for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator i = commandLineTokenizer.begin();
       i != commandLineTokenizer.end(); ++i)
    {
    if (!i->empty())
      commandLineTokens.push_back(*i);
    }

  // EMF: ugly hack that I don't really understand:
  // if the following do-nothing printf statement is compiled in, 
  // the exec after the fork works.  If you remove it, the exec fails
  // with "bad address", EFAULT
  // my guess is that  this happens because gcc is optimizing something
  // around the fork that it shouldn't optimize, but other solutions
  // like making the variable volatile didn't fix the problem.
  //printf("%s", executableAsString);
  
  pid_t pid = fork();

  if(pid < 0)
    {
    perror("Couldn't fork");
    return false;
    }
  else if(pid == 0) /* child code, run in a seperate process */
    {
    close(1);
    dup(stdOutFileDescriptor);
    close(2);
    dup(stdOutFileDescriptor);

    char **childArgv = new char*[commandLineTokens.size() + 1];
    for (int i = 0; i < commandLineTokens.size(); ++i)
      childArgv[i] = strdup(commandLineTokens[i].c_str());
    childArgv[commandLineTokens.size()] = 0;

    execv(childArgv[0], childArgv);
    // EMF: originally, this code launched the child process with:
    //  execlp(executableAsString, executableAsString);
    // but this failed with EFAULT for unknown reasons.  Sometimes
    // it would work on redhat 3 while failing on ubuntu.  
    // execv does not seem to have this problem.

    /* Code below will never happen unless exec fails */
    fprintf(stderr,"execv string=%s\n",childArgv[0]);
    perror("Couldn't exec");
    return false;
    }
  else
    {
    int waitExitStatus;
    while ( waitpid( pid, &waitExitStatus, 0 ) < 0 && errno == EINTR );
    if (WIFEXITED(waitExitStatus))
      {
      return true;
      }
    else
      {
      return false;
      }
    }
#else
  SECURITY_ATTRIBUTES saAttr;

  //ensure the child process takes the std in/out from its parents
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  HANDLE handleFileOut = CreateFile( stdOutFileName, GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, &saAttr, OPEN_ALWAYS, NULL, NULL );
  if ( !handleFileOut )
    {
#if !defined(UNIX) && !defined(__linux__)
    errorString = FetchLastErrorMessage();
#endif
    return false;
    }

  ::SetFilePointer(handleFileOut, 0, 0, FILE_END);
  //launch the child process
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;

  ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
  ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdError = handleFileOut;
  siStartInfo.hStdOutput = handleFileOut;
  siStartInfo.hStdInput = NULL;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  // CreateProcess command line parameter must be writable
  size_t cmdLineSize = strlen(commandLine) + 1;
  std::unique_ptr<char[]> cmdLine(new char[cmdLineSize]);
  memcpy(cmdLine.get(), commandLine, cmdLineSize);
  BOOL success = CreateProcess(NULL, cmdLine.get(), NULL, NULL,
                               TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &siStartInfo,
                               &piProcInfo);

  if (!success)
    {
#if !defined(UNIX) && !defined(__linux__)
    errorString = FetchLastErrorMessage();
#endif    
    CloseHandle(handleFileOut);
    return false;
    }
  else
    {
    DWORD procExitCode;
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);
    GetExitCodeProcess(piProcInfo.hProcess, &procExitCode);
    // explicitly close handles to the child process and its primary thread.
    CloseHandle(handleFileOut);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    return (procExitCode == 0);
    }
#endif
  }
