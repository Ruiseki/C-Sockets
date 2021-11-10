TASKKILL /F /IM client.exe
REN client.exe oldclient.exe
REN newClient.exe client.exe
START client.exe delete
exit