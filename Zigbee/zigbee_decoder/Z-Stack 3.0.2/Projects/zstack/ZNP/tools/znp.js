/**************************************************************************************************
  Filename:       znp.js
  Revised:        $Date: 2014-11-19 13:29:24 -0800 (Wed, 19 Nov 2014) $
  Revision:       $Revision: 41175 $

  Description:    This file is a JScript file that can be run by the Windows Script Host,
                  which is installed by default on Windows XP SP2 and later. The script acts on
                  the arguments to do the tedious post-processing for the ZNP builds.


  Copyright 2010-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/


var ForReading = 1;
var ForWriting = 2;
var fPopMask = "znp-popmask.txt";


// Get the mask of user disabled popups.
function get_popmask() {
  var fso = new ActiveXObject("Scripting.FileSystemObject");

  if (fso.FileExists(fPopMask)) {
    var fin = fso.OpenTextFile(fPopMask, ForReading);
    var mask = fin.Read(4);  // A 0/1 string is expected as the 1st 4 bytes of the 1st line.
    fin.Close();
    return (mask);
  }

  return "0000";
}


// Set the mask of user disabled popups.
function set_popmask(mask) {
  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var fout = fso.CreateTextFile(fPopMask, true);

  fout.WriteLine(mask);
  fout.WriteLine("The first byte of this file is a bit mask that forces the ZNP post processing");
  fout.WriteLine("to run popless for the corresponding build types.");
  fout.WriteLine("The bit mask is updated when the 'Cancel' button is pressed on a pop-up");
  fout.WriteLine("Simply delete this file to get all pop-ups back.");
  fout.Close();
}


// Generate an informational popup according to the build type.
function popup(idx, fName, pName) {
  var popmask = get_popmask();

  // If the user had requested to suppress popups for this build type, just return.
  if (popmask.substr(idx, 1) != 0) {
    return;
  }

  var WshShell = WScript.CreateObject("WScript.Shell");
  var msg = "The output file can be found here:\n" + pName + fName;

  switch (idx) {
  case 1:
    msg += "\nThe .hex image is intended to be programmed and verified by a tool such as\
            \nthe TI SmartRF Programmer.";
    break;
  case 2:
    msg += "\nThe .bin image is intended to be programmed and verified by a tool that implements\
            \nthe SBL protocol such as the SBDemo.exe.\
            \nNOTE: The SBL must already be programmed into the target device, usually by having\
            \n      already programmed the CC253xZNP-Prod.hex with a programming tool.";
    break;
  case 3:
    msg += "\nThe .hex image is intended to be programmed and verified by a tool such as the TI SmartRF Programmer.\
            \nNOTE: The checksum and shadow are forced to be equal, so external verification is crucial.";
    break;
  }

  // Show an info message that dimisses itself after 30 seconds and offers an OK and Cancel button.
  var BtnCode = WshShell.Popup(msg, 30, fName + " is in the dev!", 1 + 64);

  // If the user presses the 'Cancel' button, create a local file to suppress future pop-ups.
  if (BtnCode == 2) {
    var newmask = popmask.substring(0, idx) + 1 + popmask.substring(idx+1, 3);
    set_popmask(newmask);
  }
}


// void main(void)
{
  var buildName = WScript.Arguments(0);
  var targetFolder;
  switch(buildName.substr(0,6))
  {
    case "CC2530":
    case "CC2531":
      targetFolder = "CC253x\\"
      break;
    case "CC2538":
      targetFolder = "CC2538\\"
      break;
    default:
      break;
  }
  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var pZNP = fso.GetFile("znp.js");
  var fOut = buildName.substring(0, 6) + "ZNP" + buildName.substring(6, 11);
  var pOut = pZNP.Path.substr(0, pZNP.Path.length-12) + targetFolder + "dev";
  var pIn = pZNP.Path.substr(0, pZNP.Path.length-12);
  pIn += (targetFolder + buildName + "\\Exe\\");
  // remove this
  //var WshShell = WScript.CreateObject("WScript.Shell");
  //var fuck = WshShell.Popup(pIn, 90, pIn, 1 + 64);
  // remove this
  var fIn, fSBL;
  var buildType;
  var tgtType;

  if (!fso.FolderExists(pOut)) {
    fso.CreateFolder(pOut);
  }
  pOut += "\\";

  switch (buildName.substr(7)) {
  case "Debug":
    WScript.Quit(0);  // Nothing to do for a Debug build.
    break;
  case "TestHex":
    buildType = 1;
    fOut += ".hex";
    break;
  case "ZNP-without-SBL":
    fIn = buildName.substring(0, 6) + ".sim"
    fOut = buildName.substring(0, 6) + "ZNP-without-SBL";
    fOut += ".bin";
    buildType = 2;
    break;
  case "GW-ProdSBL":
  case "GP-ProdSBL":
    fIn = buildName.substring(0, 9) + ".sim";
    fOut = buildName.substring(0, 9) + "-ZNP";
    fOut += ".bin";
    buildType = 2;
    break;
  case "ZNP-with-SBL":
    fIn = buildName.substring(0, 6) + ".a51";
    fOut = buildName.substring(0, 6) + "ZNP-with-SBL";
    fOut += ".hex";
    buildType = 3;
    break;
  case "GW-ProdHex":
  case "GP-ProdHex":
    fIn = buildName.substring(0, 9) + ".a51";
    fOut = buildName.substring(0, 9) + "-ZNP";
    fOut += ".hex";
    buildType = 3;
    break;
  case "NP-with-SBL":
    fIn = buildName.substring(0, 6) + ".hex";
    fOut = buildName.substring(0, 6) + "ZNP-with-SBL";
    fOut += ".hex";
    buildType = 3;
    break;
  default:
    // Add handling of the Release Production .hex files.
    if ((buildName.substr(6, 3) == "Rel") || (buildName.substr(6, 3) == "-MK"))
    {
      pOut = pZNP.Path.substr(0, pZNP.Path.length-12) + "bin\\";
      fIn = buildName.substring(0, 9) + ".a51"
      if (buildName.substr(6, 3) == "Rel")
      {
        fOut = buildName.substring(0, 6) + "ZNP" + buildName.substr(9) + ".hex";
      }
      else
      {
        fOut = buildName.substring(0, 6) + "-MK" + buildName.substr(9) + ".hex";
      }
      buildType = 3;
    }
    else
    {
      WScript.Quit(0);  // Nothing to do for a Debug build.
    }
    break;
  }

  switch (buildName.substr(5, 1)) {
  case "0":
    tgtType = 0;
    if (buildName.substr(6, 3) == "-MK")
    {
      fSBL = pZNP.Path.substr(0, pZNP.Path.length-12) + targetFolder +"bin\\CC2530-MK-SB.hex";
    }
    else
    {
      fSBL = pZNP.Path.substr(0, pZNP.Path.length-12) + targetFolder + "bin\\CC2530SB.hex";
    }
    break;
  case "1":
    tgtType = 1;
    fSBL = pZNP.Path.substr(0, pZNP.Path.length-12) + targetFolder + "bin\\CC2531SB.hex";
    break;
  case "8":
    tgtType = 8;
    fSBL = pZNP.Path.substr(0, pZNP.Path.length-12) + targetFolder + "bin\\CC2538_SBL_UART.hex";
    break;
  }

  switch (buildType) {
  case 1:
    fso.CopyFile(pIn+fOut, pOut+fOut, true);
    break;
  case 2:
    fso.CopyFile(pIn+fIn, "tmp.sim", true);
    var WshShell = new ActiveXObject("WScript.Shell");
    // Invoke and wait for the binary file conversion tool to finish.
    WshShell.Run("sim2bin.exe tmp.sim tmp.bin", 8, true);
    fso.CopyFile("tmp.bin", pOut+fOut, true);
    fso.DeleteFile("tmp.bin");
    fso.DeleteFile("tmp.sim");
    break;
  case 3:
    var fin = fso.OpenTextFile(fSBL, ForReading)
    var fout = fso.CreateTextFile(pOut+fOut, true);
    var line = new Array(3);
    var rIdx = 2;
    var wIdx = 0;

    // Need to throw away the last two lines since a valid .hex file will be appended.
    line[0] = fin.ReadLine();
    line[1] = fin.ReadLine();
    while (1)
    {
      fout.WriteLine(line[wIdx]);
      line[rIdx] = fin.ReadLine();
      if (fin.AtEndOfStream)
      {
        break;
      }
      rIdx = (rIdx+1) % 3;
      wIdx = (wIdx+1) % 3;
    }

    fin.Close();
    fin = fso.OpenTextFile(pIn+fIn, ForReading)

    while (!fin.AtEndOfStream)
    {
      var s = fin.ReadLine();
      fout.WriteLine(s);
    }

    fin.Close();
    fout.Close();
    break;
  }

  popup(buildType, fOut, pOut);
}

