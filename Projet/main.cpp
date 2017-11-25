#pragma warning (disable:4786)
#include <string>
#include <iostream>
#include <windows.h>
#include <time.h>
#include "constants.h"
#include "misc/utils.h"
#include "Time/PrecisionTimer.h"
#include "Resource.h"
#include "misc/windowutils.h"
#include "misc/Cgdi.h"
#include "Debug/DebugConsole.h"
#include "Raven_UserOptions.h"
#include "Raven_Game.h"
#include "lua/Raven_Scriptor.h"


//need to include this for the toolbar stuff
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#define DB_OK 1 // Useful for the parameters windows

//--------------------------------- Globals ------------------------------
//------------------------------------------------------------------------

char* g_szApplicationName = "Raven";
char*	g_szWindowClassName = "MyWindowClass";

Raven_Game* g_pRaven;

HINSTANCE hinst;

UINT human;				// 0 : no human agent. 1 : human agent.
UINT mode;				// current mode used
UINT grenades;			// 0 : no grenade spawn. 1 : grenade spawn available.
UINT learning_bot;		// 0 : no learning bot. 1 : a learning bot.
UINT strategy_j1;
UINT strategy_j2;
UINT strategy_t1;
UINT strategy_t2;

// Maybe useful
BOOL APIENTRY Dialog1Proc(HWND, UINT, WPARAM, LPARAM);

//---------------------------- WindowProc ---------------------------------
//	
//	This is the callback function which handles all the windows messages
//-------------------------------------------------------------------------

LRESULT CALLBACK WindowProc (HWND   hwnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam)
{
 
   //these hold the dimensions of the client window area
	 static int cxClient, cyClient; 

	 //used to create the back buffer
   static HDC		hdcBackBuffer;
   static HBITMAP	hBitmap;
   static HBITMAP	hOldBitmap;

      //to grab filenames
   static TCHAR   szFileName[MAX_PATH],
                  szTitleName[MAX_PATH];

   // To get information about the mouse
   MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT *)lParam;

    switch (msg)
    {
	
		//A WM_CREATE msg is sent when your application window is first
		//created
    case WM_CREATE:
      {
		// initial parameters
		human = 0;
		grenades = 0;
		learning_bot = 0;

		// Ask user to enter informations for the application
		if (DialogBox(hinst, "DIALOG1", hwnd, (DLGPROC)Dialog1Proc) == DB_OK)
			InvalidateRect(hwnd, NULL, TRUE);

         //to get get the size of the client window first we need  to create
         //a RECT and then ask Windows to fill in our RECT structure with
         //the client window size. Then we assign to cxClient and cyClient 
         //accordingly
		RECT rect;
		GetClientRect(hwnd, &rect);
		cxClient = rect.right;
		cyClient = rect.bottom;

         //seed random number generator
         srand((unsigned) time(NULL));

         
         //---------------create a surface to render to(backbuffer)

         //create a memory device context
         hdcBackBuffer = CreateCompatibleDC(NULL);

         //get the DC for the front buffer
         HDC hdc = GetDC(hwnd);

         hBitmap = CreateCompatibleBitmap(hdc,
                                          cxClient,
                                          cyClient);

			  
         //select the bitmap into the memory device context
		 hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

         //don't forget to release the DC
         ReleaseDC(hwnd, hdc);  

		 //TestTeamSimple
		 //mode = TEAM_MATCH;
		 strategy_t1 = 0;
		 strategy_t2 = 0;
         //create the game
         g_pRaven = new Raven_Game(mode, human, grenades, learning_bot, strategy_j1, strategy_j2,
			 strategy_t1, strategy_t2);

		 debug_con << "strategy t1 !" << strategy_t1 << "";

        //make sure the menu items are ticked/unticked accordingly
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SHOW_NAVGRAPH, UserOptions->m_bShowGraph);
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SHOW_PATH, UserOptions->m_bShowPathOfSelectedBot);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_IDS, UserOptions->m_bShowBotIDs);
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SMOOTH_PATHS_QUICK, UserOptions->m_bSmoothPathsQuick);
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SMOOTH_PATHS_PRECISE, UserOptions->m_bSmoothPathsPrecise);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_HEALTH, UserOptions->m_bShowBotHealth);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_TARGET, UserOptions->m_bShowTargetOfSelectedBot);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_FOV, UserOptions->m_bOnlyShowBotsInTargetsFOV);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_SCORES, UserOptions->m_bShowScore);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_GOAL_Q, UserOptions->m_bShowGoalsOfSelectedBot);
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SHOW_INDICES, UserOptions->m_bShowNodeIndices);
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_SENSED, UserOptions->m_bShowOpponentsSensedBySelectedBot);

      }

      break;

    case WM_KEYUP:
      {
        switch(wParam)
        {
         case VK_ESCAPE:
          {
            SendMessage(hwnd, WM_DESTROY, NULL, NULL);
          }
          
          break;

         case 'P':
			debug_con << "PAUSE !" << "";
           g_pRaven->TogglePause();

           break;

         case '1':

           g_pRaven->ChangeWeaponOfPossessedBot(type_blaster);

           break;

         case '2':

           g_pRaven->ChangeWeaponOfPossessedBot(type_shotgun);

           break;
           
         case '3':

           g_pRaven->ChangeWeaponOfPossessedBot(type_rocket_launcher);

           break;

         case '4':

           g_pRaven->ChangeWeaponOfPossessedBot(type_rail_gun);

           break;

		 case '5':

			g_pRaven->ChangeWeaponOfPossessedBot(type_grenade);

			break;

         case 'X':

           g_pRaven->ExorciseAnyPossessedBot();

           break;


         case VK_UP:

           g_pRaven->AddBots(1); break;

         case VK_DOWN:

           g_pRaven->RemoveBot(); break;
           

        }
      }

    break;

	case WM_KEYDOWN:
	{
		
	}

	break;

    case WM_LBUTTONDOWN:
    {
      g_pRaven->ClickLeftMouseButton(MAKEPOINTS(lParam));
    }
    
    break;

   case WM_RBUTTONDOWN:
    {
      g_pRaven->ClickRightMouseButton(MAKEPOINTS(lParam));
    }
    
    break;

    case WM_MOUSEWHEEL:
	{
		WPARAM fwKeys = GET_KEYSTATE_WPARAM(wParam);
		WPARAM zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		//WPARAM zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		debug_con << "Changement d'arme !" << "";
		g_pRaven->ScrollMouseButton(zDelta == 120);

		//MScrollUp = HIWORD(pMouseStruct->mouseData) == 120;
		//g_pRaven->ScrollMouseButton(MScrollUp);
	}

	break;

    case WM_COMMAND:
    {

     switch(wParam)
      {
      

      case IDM_GAME_LOAD:
          
          FileOpenDlg(hwnd, szFileName, szTitleName, "Raven map file (*.map)", "map");

          debug_con << "Filename: " << szTitleName << "";

          if (strlen(szTitleName) > 0)
          {
            g_pRaven->LoadMap(szTitleName);
          }

          break;

      case IDM_GAME_ADDBOT:

          g_pRaven->AddBots(1);
          
          break;

      case IDM_GAME_REMOVEBOT:
          
          g_pRaven->RemoveBot();

          break;

      case IDM_GAME_PAUSE:
          
          g_pRaven->TogglePause();

          break;



      case IDM_NAVIGATION_SHOW_NAVGRAPH:

        UserOptions->m_bShowGraph = !UserOptions->m_bShowGraph;

        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SHOW_NAVGRAPH, UserOptions->m_bShowGraph);

        break;
        
      case IDM_NAVIGATION_SHOW_PATH:

        UserOptions->m_bShowPathOfSelectedBot = !UserOptions->m_bShowPathOfSelectedBot;

        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SHOW_PATH, UserOptions->m_bShowPathOfSelectedBot);

        break;

      case IDM_NAVIGATION_SHOW_INDICES:

        UserOptions->m_bShowNodeIndices = !UserOptions->m_bShowNodeIndices;

        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SHOW_INDICES, UserOptions->m_bShowNodeIndices);

        break;

      case IDM_NAVIGATION_SMOOTH_PATHS_QUICK:

        UserOptions->m_bSmoothPathsQuick = !UserOptions->m_bSmoothPathsQuick;
        UserOptions->m_bSmoothPathsPrecise = false;
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SMOOTH_PATHS_PRECISE, UserOptions->m_bSmoothPathsPrecise);
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SMOOTH_PATHS_QUICK, UserOptions->m_bSmoothPathsQuick);

        break;

      case IDM_NAVIGATION_SMOOTH_PATHS_PRECISE:

        UserOptions->m_bSmoothPathsPrecise = !UserOptions->m_bSmoothPathsPrecise;
        UserOptions->m_bSmoothPathsQuick = false;
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SMOOTH_PATHS_QUICK, UserOptions->m_bSmoothPathsQuick);
        CheckMenuItemAppropriately(hwnd, IDM_NAVIGATION_SMOOTH_PATHS_PRECISE, UserOptions->m_bSmoothPathsPrecise);

        break;

      case IDM_BOTS_SHOW_IDS:

        UserOptions->m_bShowBotIDs = !UserOptions->m_bShowBotIDs;

        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_IDS, UserOptions->m_bShowBotIDs);

        break;

      case IDM_BOTS_SHOW_HEALTH:

        UserOptions->m_bShowBotHealth = !UserOptions->m_bShowBotHealth;
        
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_HEALTH, UserOptions->m_bShowBotHealth);

        break;

      case IDM_BOTS_SHOW_TARGET:

        UserOptions->m_bShowTargetOfSelectedBot = !UserOptions->m_bShowTargetOfSelectedBot;
        
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_TARGET, UserOptions->m_bShowTargetOfSelectedBot);

        break;

      case IDM_BOTS_SHOW_SENSED:

        UserOptions->m_bShowOpponentsSensedBySelectedBot = !UserOptions->m_bShowOpponentsSensedBySelectedBot;
        
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_SENSED, UserOptions->m_bShowOpponentsSensedBySelectedBot);

        break;


      case IDM_BOTS_SHOW_FOV:

        UserOptions->m_bOnlyShowBotsInTargetsFOV = !UserOptions->m_bOnlyShowBotsInTargetsFOV;
        
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_FOV, UserOptions->m_bOnlyShowBotsInTargetsFOV);

        break;

      case IDM_BOTS_SHOW_SCORES:

        UserOptions->m_bShowScore = !UserOptions->m_bShowScore;
        
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_SCORES, UserOptions->m_bShowScore);

        break;

      case IDM_BOTS_SHOW_GOAL_Q:

        UserOptions->m_bShowGoalsOfSelectedBot = !UserOptions->m_bShowGoalsOfSelectedBot;
        
        CheckMenuItemAppropriately(hwnd, IDM_BOTS_SHOW_GOAL_Q, UserOptions->m_bShowGoalsOfSelectedBot);

        break;

      }//end switch
    }

    
    case WM_PAINT:
      {
 		       
         PAINTSTRUCT ps;
          
         BeginPaint (hwnd, &ps);

        //fill our backbuffer with white
         BitBlt(hdcBackBuffer,
                0,
                0,
                cxClient,
                cyClient,
                NULL,
                NULL,
                NULL,
                WHITENESS);
         
		 // Use to move the human player
		 if (human) {
			 Vector2D direction = Vector2D(0, 0);
			 if (GetAsyncKeyState('Z')) {
				 direction += Vector2D(0, -5);
			 }
			 if (GetAsyncKeyState('Q')) {
				 direction += Vector2D(-5, 0);
			 }
			 if (GetAsyncKeyState('S')) {
				 direction += Vector2D(0, 5);
			 }
			 if (GetAsyncKeyState('D')) {
				 direction += Vector2D(5, 0);
			 }
			 g_pRaven->MoveToward(direction);
		 }

		 // Use for render the game
         gdi->StartDrawing(hdcBackBuffer);

         g_pRaven->Render();

         gdi->StopDrawing(hdcBackBuffer);


         //now blit backbuffer to front
			   BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY); 
          
         EndPaint (hwnd, &ps);

      }

      break;

    //has the user resized the client area?
		case WM_SIZE:
		  {
        //if so we need to update our variables so that any drawing
        //we do using cxClient and cyClient is scaled accordingly
			  cxClient = LOWORD(lParam);
			  cyClient = HIWORD(lParam);

        //now to resize the backbuffer accordingly. First select
        //the old bitmap back into the DC
			  SelectObject(hdcBackBuffer, hOldBitmap);

        //don't forget to do this or you will get resource leaks
        DeleteObject(hBitmap); 

			  //get the DC for the application
        HDC hdc = GetDC(hwnd);

			  //create another bitmap of the same size and mode
        //as the application
        hBitmap = CreateCompatibleBitmap(hdc,
											  cxClient,
											  cyClient);

			  ReleaseDC(hwnd, hdc);
			  
			  //select the new bitmap into the DC
        SelectObject(hdcBackBuffer, hBitmap);

      }

      break;
          
		 case WM_DESTROY:
			 {

         //clean up our backbuffer objects
         SelectObject(hdcBackBuffer, hOldBitmap);

         DeleteDC(hdcBackBuffer);
         DeleteObject(hBitmap); 
         

         // kill the application, this sends a WM_QUIT message  
				 PostQuitMessage (0);
			 }

       break;

     }//end switch

     //this is where all the messages not specifically handled by our 
		 //winproc are sent to be processed
		 return DefWindowProc (hwnd, msg, wParam, lParam);
}

BOOL APIENTRY Dialog1Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// Settings of the comboBox mode
		SendDlgItemMessage(hDlg, ID_MODE, CB_ADDSTRING, 0, (LONG)"Deathmatch");
		SendDlgItemMessage(hDlg, ID_MODE, CB_ADDSTRING, 0, (LONG)"Team Deathmatch");
		SendDlgItemMessage(hDlg, ID_MODE, CB_ADDSTRING, 0, (LONG)"1 vs 1");
		SendDlgItemMessage(hDlg, ID_MODE, CB_SETCURSEL, mode, 0);

		// Settings of the comboBox players strategies
		SendDlgItemMessage(hDlg, ID_STRAT_J1, CB_ADDSTRING, 0, (LONG)"Burnhead");
		SendDlgItemMessage(hDlg, ID_STRAT_J1, CB_ADDSTRING, 0, (LONG)"Coward");
		SendDlgItemMessage(hDlg, ID_STRAT_J1, CB_ADDSTRING, 0, (LONG)"Camper");
		SendDlgItemMessage(hDlg, ID_STRAT_J1, CB_SETCURSEL, strategy_j1, 0);

		SendDlgItemMessage(hDlg, ID_STRAT_J2, CB_ADDSTRING, 0, (LONG)"Burnhead");
		SendDlgItemMessage(hDlg, ID_STRAT_J2, CB_ADDSTRING, 0, (LONG)"Coward");
		SendDlgItemMessage(hDlg, ID_STRAT_J2, CB_ADDSTRING, 0, (LONG)"Camper");
		SendDlgItemMessage(hDlg, ID_STRAT_J2, CB_SETCURSEL, strategy_j2, 0);

		// Settings of the comboBox team strategies
		SendDlgItemMessage(hDlg, ID_STRAT_T1, CB_ADDSTRING, 0, (LONG)"divideAndRule");
		SendDlgItemMessage(hDlg, ID_STRAT_T1, CB_ADDSTRING, 0, (LONG)"TestudoFormation");
		SendDlgItemMessage(hDlg, ID_STRAT_T1, CB_ADDSTRING, 0, (LONG)"LeaderFollowing");
		SendDlgItemMessage(hDlg, ID_STRAT_T1, CB_SETCURSEL, strategy_j1, 0);

		SendDlgItemMessage(hDlg, ID_STRAT_T2, CB_ADDSTRING, 0, (LONG)"divideAndRule");
		SendDlgItemMessage(hDlg, ID_STRAT_T2, CB_ADDSTRING, 0, (LONG)"TestudoFormation");
		SendDlgItemMessage(hDlg, ID_STRAT_T2, CB_ADDSTRING, 0, (LONG)"LeaderFollowing");
		SendDlgItemMessage(hDlg, ID_STRAT_T2, CB_SETCURSEL, strategy_j2, 0);
		
		// Settings des boutons radio
		CheckDlgButton(hDlg, ID_NO_HUMAN, BST_CHECKED);
		CheckDlgButton(hDlg, ID_NO_BOT_APPRE, BST_CHECKED);
		CheckDlgButton(hDlg, ID_NO_GRENADE, BST_CHECKED);
		return TRUE;
	}
	case WM_COMMAND:

		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// Retrieve the choice of method
			int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
				(WPARAM)0, (LPARAM)0);
			TCHAR  ListItem[256];
			(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
				(WPARAM)ItemIndex, (LPARAM)ListItem);
			//MessageBox(hDlg, (LPCWSTR)ListItem, TEXT("Item Selected"), MB_OK);

			/*if (strcmp(ListItem, "LeaderFollowing") == 0)
				LeaderFollowingField(hDlg, true);
			else
				LeaderFollowingField(hDlg, false);*/
		}

		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
				case ID_BOT_APPRE:
				{ learning_bot = 1; break; }
				case ID_NO_BOT_APPRE:
				{ learning_bot = 0; break; }
				case ID_GRENADE:
				{ grenades = 1; break; }
				case ID_NO_GRENADE:
				{ grenades = 0; break; }
				case ID_HUMAN:
				{ human = 1; break; }
				case ID_NO_HUMAN:
				{ human = 0; break; }
			}
		}
		if (LOWORD(wParam) == DB_OK || LOWORD(wParam) == IDCANCEL)
		{
			// get the behavior wanted
			mode = SendDlgItemMessage(hDlg, ID_MODE, CB_GETCURSEL, 0, 0);
			// get the number of stadard agent 
			strategy_j1 = GetDlgItemInt(hDlg, ID_STRAT_J1, NULL, FALSE);
			strategy_j2 = GetDlgItemInt(hDlg, ID_STRAT_J2, NULL, FALSE);
			// get the number of pursuiver for the leader1
			strategy_t1 = GetDlgItemInt(hDlg, ID_STRAT_T1, NULL, FALSE);
			strategy_t2 = GetDlgItemInt(hDlg, ID_STRAT_T2, NULL, FALSE);

			EndDialog(hDlg, DB_OK);
			return TRUE;
		}
	default:
		return FALSE;
	}
}

//-------------------------------- WinMain -------------------------------
//
//	The entry point of the windows program
//------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     szCmdLine, 
                    int       iCmdShow)
{
  MSG msg;
  //handle to our window
	HWND						hWnd;

 //the window class structure
	WNDCLASSEX     winclass;

  // first fill in the window class stucture
	winclass.cbSize        = sizeof(WNDCLASSEX);
	winclass.style         = CS_HREDRAW | CS_VREDRAW;
  winclass.lpfnWndProc   = WindowProc;
  winclass.cbClsExtra    = 0;
  winclass.cbWndExtra    = 0;
  winclass.hInstance     = hInstance;
  winclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  winclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winclass.hbrBackground = NULL;
  winclass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
  winclass.lpszClassName = g_szWindowClassName;
	winclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  //register the window class
  if (!RegisterClassEx(&winclass))
  {
		MessageBox(NULL, "Registration Failed!", "Error", 0);

	  //exit the application
		return 0;
  }
		

  try
  {  		 
		 //create the window and assign its ID to hwnd    
     hWnd = CreateWindowEx (NULL,                 // extended style
                            g_szWindowClassName,  // window class name
                            g_szApplicationName,  // window caption
                            WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,  // window style
                            GetSystemMetrics(SM_CXSCREEN)/2 - WindowWidth/2,
                            GetSystemMetrics(SM_CYSCREEN)/2 - WindowHeight/2,                    
                            WindowWidth,     // initial x size
                            WindowHeight,    // initial y size
                            NULL,                 // parent window handle
                            NULL,                 // window menu handle
                            hInstance,            // program instance handle
                            NULL);                // creation parameters

     //make sure the window creation has gone OK
     if(!hWnd)
     {
       MessageBox(NULL, "CreateWindowEx Failed!", "Error!", 0);
     }
     
    //make the window visible
    ShowWindow (hWnd, iCmdShow);
    UpdateWindow (hWnd);
   
    //create a timer
    PrecisionTimer timer(FrameRate);

    //start the timer
    timer.Start();

    //enter the message loop
    bool bDone = false;

    while(!bDone)
    {
      while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
      {
        if( msg.message == WM_QUIT ) 
        {
          // Stop loop if it's a quit message
	        bDone = true;
        } 

        else 
        {
          TranslateMessage( &msg );
          DispatchMessage( &msg );
        }
      }

      if (timer.ReadyForNextFrame() && msg.message != WM_QUIT)
      {
        g_pRaven->Update();
        
        //render 
        RedrawWindow(hWnd);
      }

      //give the OS a little time
      Sleep(2);
     					
    }//end while

  }//end try block

  catch (const std::runtime_error& err)
  {
    ErrorBox(std::string(err.what()));
    //tidy up
    delete g_pRaven;
    UnregisterClass( g_szWindowClassName, winclass.hInstance );
    return 0;
  }
  
 //tidy up
 UnregisterClass( g_szWindowClassName, winclass.hInstance );
 delete g_pRaven;
 return msg.wParam;
}


