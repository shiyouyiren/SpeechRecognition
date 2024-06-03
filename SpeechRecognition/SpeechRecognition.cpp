#include <iostream> 
#include <stdlib.h>
#include <speechapi_cxx.h>

#include <string>
#include <atlstr.h>
#include <stdio.h>

using namespace std;

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;
using namespace Microsoft::CognitiveServices::Speech::Translation;


BOOL CreateString_InitWithUTF8Text(CString& str, char* pUTF8Text)
{
	if (NULL == pUTF8Text)
	{
		return FALSE;
	}

	int  unicodeLen = ::MultiByteToWideChar(CP_UTF8,
		0,
		pUTF8Text,
		-1,
		NULL,
		0);

	wchar_t* pUnicode = new  wchar_t[unicodeLen + 1];
	if (NULL == pUnicode)
	{
		return FALSE;
	}

	MultiByteToWideChar(CP_UTF8, 0, pUTF8Text, -1, (LPWSTR)pUnicode, unicodeLen);

	str = pUnicode;

	delete[]pUnicode;
	return TRUE;
}


int main()
{
	// This example requires environment variables named "SPEECH_KEY" and "SPEECH_REGION"
	//auto speechKey = GetEnvironmentVariable("SPEECH_KEY");
	//auto speechRegion = GetEnvironmentVariable("SPEECH_REGION");

	string speechKey = "57149b475b6d4fab955f96408eb49f9e";
	string speechRegion = "southeastasia";

	if ((size(speechKey) == 0) || (size(speechRegion) == 0)) {
		std::cout << "Please set both SPEECH_KEY and SPEECH_REGION environment variables." << std::endl;
		return -1;
	}

	auto speechConfig = SpeechConfig::FromSubscription(speechKey, speechRegion);

	//speechConfig->SetSpeechRecognitionLanguage("en-US");
	speechConfig->SetSpeechRecognitionLanguage("zh-CN");
	speechConfig->SetSpeechSynthesisLanguage("en-US");
	speechConfig->SetSpeechSynthesisLanguage("zh-CN-sichuan");

	auto audioConfig = AudioConfig::FromDefaultMicrophoneInput();
	auto speechRecognizer = SpeechRecognizer::FromConfig(speechConfig, audioConfig);

	std::cout << "Speak into your microphone.\n";
	speechRecognizer->StartContinuousRecognitionAsync().get();

	promise<void> recognitionEnd;
	// Waits for recognition end.
	//recognitionEnd.get_future().get();
	
	try
	{
		speechRecognizer->Recognizing.Connect([](const SpeechRecognitionEventArgs& e)
			{
				string strText = e.Result->Text;
				cout << "Recognizing:" << strText << std::endl;
				CString cstrText;
				CreateString_InitWithUTF8Text(cstrText, (char*)strText.c_str());
				cout << "Recognizing:" << cstrText.GetBuffer() << std::endl;
			});

		cout << "Recognizing end" << endl;

		speechRecognizer->Recognized.Connect([](const SpeechRecognitionEventArgs& e)
			{
				if (e.Result->Reason == ResultReason::RecognizedSpeech)
				{
					string strText = e.Result->Text;
					cout << "RECOGNIZED: Text=" << strText
						<< " (text could not be translated)" << std::endl;

					CString cstrText;
					CreateString_InitWithUTF8Text(cstrText, (char*)strText.c_str());
					cout << "RECOGNIZED: Text=" << cstrText.GetBuffer()
						<< " (text could not be translated)" << std::endl;
				}
				else if (e.Result->Reason == ResultReason::NoMatch)
				{
					cout << "NOMATCH: Speech could not be recognized." << std::endl;
				}
			});

		cout << "Recognized end" << endl;

		speechRecognizer->Canceled.Connect([&recognitionEnd](const SpeechRecognitionCanceledEventArgs& e)
			{
				cout << "CANCELED: Reason=" << (int)e.Reason << std::endl;
				if (e.Reason == CancellationReason::Error)
				{
					cout << "CANCELED: ErrorCode=" << (int)e.ErrorCode << "\n"
						<< "CANCELED: ErrorDetails=" << e.ErrorDetails << "\n"
						<< "CANCELED: Did you set the speech resource key and region values?" << std::endl;

					recognitionEnd.set_value(); // Notify to stop recognition.
				}
			});

		cout << "Canceled end" << endl;
		speechRecognizer->SessionStopped.Connect([&recognitionEnd](const SessionEventArgs& e)
			{
				cout << "Session stopped." << endl;
				//recognitionEnd.set_value(); // Notify to stop recognition.
			});

		cout << "SessionStopped" << endl;

		cout << "get_future begin" << endl;
		recognitionEnd.get_future().get();
		cout << "get_future end" << endl;

		// Stops recognition.
		speechRecognizer->StopContinuousRecognitionAsync().get();
		cout << "close normal" << endl;
	}
	catch (exception e)
	{
		cout << "\nexception=" << e.what() << endl;
	}
	cout << "Please press a key to continue.\n";
	getchar();
	return 0;
}
