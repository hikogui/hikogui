

void dialog_output(char const *title, char const *message)
{
    @autoreleasepool {
        NSString *title_s = [NSString stringWithCString:title encoding:NSUTF8StringEncoding];
        NSString *message_s = [NSString stringWithCString:message encoding:NSUTF8StringEncoding];

        auto *alert = [[NSAlert alloc] init];
        [alert setMessageText:title_s];
        [alert setInformativeText:message_s];
        [alert addButtonWithTitle:@"OK"];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
    }
}
