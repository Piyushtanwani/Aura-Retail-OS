/*
 * ============================================================================
 *  AURA RETAIL OS — Command Interface (Command Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Command Pattern
 *
 *  The Command pattern encapsulates a request as an object, thereby letting 
 *  you parameterize clients with different requests, queue or log requests, 
 *  and support undoable operations.
 *
 *  All major kiosk operations (Purchase, Refund, Restock) implement this
 *  interface.
 *
 * ============================================================================
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <string>

class Command {
public:
    virtual ~Command() = default;
    
    // Execute the command
    virtual void execute() = 0;
    
    // Get a description of the command for logging
    virtual std::string getDescription() const = 0;
};

#endif // COMMAND_H
