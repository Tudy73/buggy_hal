#include "HAL9000.h"
#include "keyboard.h"
#include "keyboard_utils.h"
#include "display.h"

#define CMD_SHELL       ">>"
#define CMD_SHELL_SIZE  (sizeof(CMD_SHELL))

KEYCODE
getch(
    void
    )
{
    KEYCODE result = KEY_UNKNOWN;

    do 
    {
        result = KeyboardWaitForKey();
        KeyboardDiscardLastKey();
    } while (KEY_UNKNOWN == result);

    

    return result;
}

void
gets_s(
    OUT_WRITES_Z(BufferSize)    char*       Buffer,
    IN                          DWORD       BufferSize,
    OUT                         DWORD*      UsedSize
    )
{
    DWORD i;
    DWORD maxBufferSize;
    KEYCODE key;
    char c;
    SCREEN_POSITION cursorPosition;

    i = 0;
    key = KEY_UNKNOWN;
    c = 0;

    cursorPosition.Line = LINES_PER_SCREEN - 1;
    cursorPosition.Column = CMD_SHELL_SIZE;

    ASSERT( NULL != UsedSize );
    ASSERT( NULL != Buffer );
    ASSERT( BufferSize > 1 );

    // we cannot write more than a line or then the buffer we have
    // we add sizeof('\0') because we do not need to print the NULL terminator :)
    maxBufferSize = min(BufferSize, CHARS_PER_LINE - CMD_SHELL_SIZE + 1);

    // zero the buffer
    memzero(Buffer, BufferSize);

#pragma warning(suppress:4127)
    while (TRUE)
    {
        DispClearLine(LINES_PER_SCREEN - 1);
        DispSetCursor(cursorPosition, CYAN_COLOR);

        // display command shell
        DispPutBufferColor(CMD_SHELL, LINES_PER_SCREEN - 1, 0, BLUE_COLOR);

        // warning 6054: String 'Buffer' might not be zero-terminated
        // the memzero call outside the while loop zero-terminates the string
#pragma warning(suppress: 6054)
        DispPutBufferColor(Buffer, LINES_PER_SCREEN - 1, CMD_SHELL_SIZE, BRIGHT_CYAN_COLOR);

        key = getch();
        ASSERT_INFO(KEY_UNKNOWN != key, "getch can't return when there is no valid key\n");

        if (KEY_RETURN == key)
        {
            // enter was pressed
            break;
        }

        if (KEY_BACKSPACE == key)
        {
            // delete a key
            if (0 != i)
            {
                // if i is already 0 nothing to delete
                cursorPosition.Column--;
                --i;
                Buffer[i] = '\0';
            }

            // go to the next iteration
            continue;
        }
        
        if (KEY_LEFT == key)
        {
            // move cursor to the left
            if (0 != i)
            {
                // if i is already 0 we have nowhere to go
                cursorPosition.Column--;
                --i;
            }

            // go to the next iteration
            continue;
        }

        if (KEY_RIGHT == key)
        {
            // move cursor to the right
            if (i < maxBufferSize - 2 )
            {
                // we use -2 because if we move the cursor we must
                // be able to write a character afterward
                
                // [BUFFER_SIZE - 2][CHAR AFTER CURSOR MOVEMENT][\0]

                cursorPosition.Column++;

                // if the character in the buffer differs from the NULL terminator we need to
                // preserve its value
                if ('\0' == Buffer[i])
                {
                    // need to set space in buffer, else it will still
                    // be NULL terminated and we can't write anything to it
                    Buffer[i] = ' ';
                }
                ++i;
            }

            // go to the next iteration
            continue;
        }

        // if we're here we might have an ASCII character
        c = KeyboardKeyToAscii(key);
        if (0 != c)
        {
            // we have an ASCII character
            Buffer[i] = c;
            ++i;
            cursorPosition.Column++;
        }

        // we use BufferSize - 1 because we append a NULL terminator
        if (i >= maxBufferSize - 1)
        {
            // we cannot write anymore in the buffer
            break;
        }
    }

    *UsedSize = i;
}