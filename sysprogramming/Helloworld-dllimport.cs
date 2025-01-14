using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Text;

[DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
static extern bool WriteConsole(IntPtr hConsoleOutput, string lpBuffer, 
                uint nNumberOfCharsToWrite, out uint lpNumberOfCharsWritten,
                IntPtr lpReserved);

[DllImport("kernel32.dll", SetLastError = true)]
static extern IntPtr GetStdHandle (int nStdHandle);
[DllImport("kernel32.dll")]
static extern uint GetLastError();


const int STD_OUTPUT_HANDLE = -11;

IntPtr stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

if (stdHandle == IntPtr.Zero){
    Console.WriteLine("Failed to get standard output handle");
    return;
}

string output = "Hello, Sysprog!";
uint charsWritten;

[DllImport("kernel32.dll")]
static extern uint FormatMessage(uint dwFlags, IntPtr lpSource, uint dwMessageId,
    uint dwLanguageId, StringBuilder lpBuffer, int nSize, IntPtr Arguments);

if (!WriteConsole(stdHandle, output, (uint) output.Length, out charsWritten, IntPtr.Zero)){
    var lastError = Marshal.GetLastWin32Error();
    int bufferSize = 256;
    var errorBuffer = new StringBuilder(bufferSize);
    var res = FormatMessage(0x1000, IntPtr.Zero, (uint) lastError, 0, errorBuffer, bufferSize, IntPtr.Zero);
    if (res != IntPtr.Zero)
    {
        var formattedError =  errorBuffer.ToString();
        Console.WriteLine(formattedError);
    }
}
