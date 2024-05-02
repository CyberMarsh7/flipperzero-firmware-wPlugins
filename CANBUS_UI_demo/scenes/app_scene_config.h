//---------------------------------------------------------------------
//  Here you gonna put all the scenes you wanna use, they have to be in
//  order.
//
//  For any scene you "add", you have to create 3 functions in every scene,
//  they can be on one file or can be in differents files but must be included
//  in the program
//
//  The name of the functions will be given in the app_scene_functions.c
//  there is a code that defines the names and the functions
//---------------------------------------------------------------------

ADD_SCENE(app, Menu, MainMenu)

ADD_SCENE(app, SniffingTest, sniffingTestOption)
ADD_SCENE(app, BoxSniffing, boxSniffing)

ADD_SCENE(app, SenderTest, senderTest)
ADD_SCENE(app, send_message, SendMessage)
ADD_SCENE(app, warning_log, warningLogSender)
ADD_SCENE(app, id_list, idListOption)
ADD_SCENE(app, input_text, inputTextOption)

ADD_SCENE(app, read_logs, readLogs)

ADD_SCENE(app, Settings, settingsOption)