COMPILER = g++
COMPILERFLAGS = -ansi -Wall -rdynamic -pthread
PROJECT = smonline
OBJECTS = AssertionHandler.o Backtrace.o BacktraceNames.o concursor.o ConstructCount.o crashDefines.o CrashHandler.o CrashHandlerChild.o EmergencyShutdown.o ezMySQL.o ezMySQLConstants.o ezsockets.o ezSQL.o GetSysInfo.o JudgeWindow.o LinuxThreadHelpers.o MercuryString.o MercuryThreads.o PacketFunctions.o RoundInfo.o RunningUnderValgrind.o SignalHandler.o SimpleLog.o SMLanPlayers.o smonline.o SMOnlineClient.o SMOnlineGameRoom.o SMOnlineRoom.o SMOnlineServer.o SMOnlineStats.o SMOnlineSQL.o SQLfunctions.o MercuryFiles.o MercuryINI.o

${PROJECT}: ${OBJECTS}
	${COMPILER} ${COMPILERFLAGS} -ldl -lpthread -o ${PROJECT} ${OBJECTS}

concursor.o: concursor.cpp
	${COMPILER} ${COMPILERFLAGS} -c concursor.cpp

ConstructCount.o: ConstructCount.cpp
	${COMPILER} ${COMPILERFLAGS} -c ConstructCount.cpp

ezsockets.o: ezsockets.cpp
	${COMPILER} ${COMPILERFLAGS} -c ezsockets.cpp

MercuryThreads.o: MercuryThreads.cpp
	${COMPILER} ${COMPILERFLAGS} -c MercuryThreads.cpp

JudgeWindow.o: JudgeWindow.cpp
	${COMPILER} ${COMPILERFLAGS} -c JudgeWindow.cpp

PacketFunctions.o: PacketFunctions.cpp
	${COMPILER} ${COMPILERFLAGS} -c PacketFunctions.cpp

RoundInfo.o: RoundInfo.cpp
	${COMPILER} ${COMPILERFLAGS} -c RoundInfo.cpp

SimpleLog.o: SimpleLog.cpp
	${COMPILER} ${COMPILERFLAGS} -c SimpleLog.cpp

SMLanPlayers.o: SMLanPlayers.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMLanPlayers.cpp

smonline.o: smonline.cpp
	${COMPILER} ${COMPILERFLAGS} -c smonline.cpp

SMOnlineClient.o: SMOnlineClient.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMOnlineClient.cpp

SMOnlineGameRoom.o: SMOnlineGameRoom.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMOnlineGameRoom.cpp

SMOnlineRoom.o: SMOnlineRoom.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMOnlineRoom.cpp

SMOnlineServer.o: SMOnlineServer.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMOnlineServer.cpp

SMOnlineStats.o: SMOnlineStats.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMOnlineStats.cpp

crashDefines.o: Crash/crashDefines.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/crashDefines.cpp

AssertionHandler.o: Crash/Linux/AssertionHandler.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/AssertionHandler.cpp

Backtrace.o: Crash/Linux/Backtrace.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/Backtrace.cpp

BacktraceNames.o: Crash/Linux/BacktraceNames.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/BacktraceNames.cpp

CrashHandler.o: Crash/Linux/CrashHandler.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/CrashHandler.cpp

CrashHandlerChild.o: Crash/Linux/CrashHandlerChild.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/CrashHandlerChild.cpp

#CrashHandlerInternal.o: Crash/Linux/CrashHandlerInternal.cpp
#	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/CrashHandlerInternal.cpp

EmergencyShutdown.o: Crash/Linux/EmergencyShutdown.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/EmergencyShutdown.cpp

GetSysInfo.o: Crash/Linux/GetSysInfo.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/GetSysInfo.cpp

LinuxThreadHelpers.o: Crash/Linux/LinuxThreadHelpers.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/LinuxThreadHelpers.cpp

RunningUnderValgrind.o: Crash/Linux/RunningUnderValgrind.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/RunningUnderValgrind.cpp

SignalHandler.o: Crash/Linux/SignalHandler.cpp
	${COMPILER} ${COMPILERFLAGS} -c Crash/Linux/SignalHandler.cpp

ezSQL.o: ezSQL.cpp
	${COMPILER} ${COMPILERFLAGS} -c ezSQL.cpp

ezMySQLConstants.o: ezMySQLConstants.cpp
	${COMPILER} ${COMPILERFLAGS} -c ezMySQLConstants.cpp

ezMySQL.o: ezMySQL.cpp
	${COMPILER} ${COMPILERFLAGS} -c ezMySQL.cpp

SMOnlineSQL.o: SMOnlineSQL.cpp
	 ${COMPILER} ${COMPILERFLAGS} -c SMOnlineSQL.cpp

SQLfunctions.o: SQLfunctions.cpp
	${COMPILER} ${COMPILERFLAGS} -c SQLfunctions.cpp

SMOnlineStats.o: SMOnlineStats.cpp
	${COMPILER} ${COMPILERFLAGS} -c SMOnlineStats.cpp

ezThreads.o: ezThreads.cpp
	${COMPILER} ${COMPILERFLAGS} -c ezThreads.cpp

MercuryFiles.o: MercuryFiles.cpp
	${COMPILER} ${COMPILERFLAGS} -c MercuryFiles.cpp

MercuryINI.o: MercuryINI.cpp
	${COMPILER} ${COMPILERFLAGS} -c MercuryINI.cpp

clean:
	rm -rf ${OBJECTS} ${PROJECT}

