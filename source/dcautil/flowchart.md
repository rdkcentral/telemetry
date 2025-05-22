```mermaid
flowchart TD
    Start[getDCAResultsInVector] --> Lock[Lock dcaMutex]
    
    Lock --> CheckInput{Check vecMarkerList}
    CheckInput -->|NULL| Unlock1[Unlock dcaMutex]
    Unlock1 --> Return1[Return -1]
    
    CheckInput -->|Valid| CheckProps{Check Properties\nInitialized?}
    
    CheckProps -->|No| InitProps[Initialize Properties\nlogPath & persistentPath]
    CheckProps -->|Yes| CheckCustomPath
    InitProps --> CheckCustomPath
    
    CheckCustomPath{Custom Log Path?} -->|Yes| UpdateProps[Update Properties\nwith customLogPath]
    CheckCustomPath -->|No| CreateVector
    UpdateProps --> CreateVector
    
    CreateVector[Create grepResultList Vector] --> ParseMarkers[parseMarkerList]
    
    ParseMarkers --> ProcessSeek{Check if logfile differs between iterations and open new file descriptors}
    subgraph LoopThroughMarkers[Loop Through Markers]
        ProcessSeek --> GetSeekMap[Get/Create LogSeekMap]
        GetSeekMap --> insertPCNode[T2 Marker object transformed to LegacyCode object]
        insertPCNode --> processPattern
        processPattern -->|top_log.txt| TopPattern[Process Top Pattern]
        processPattern -->|message_bus| TR181[Process TR181 Objects - Only one with skip frequency lands here]
        processPattern -->|other| LogPattern[Extract line by line from log file]
        LogPattern --> LogPattern
        TopPattern --> Transform
        TR181 --> Transform
        LogPattern --> Transform[Transform from legacy code object ]
        Transform --> AddVector

    end
    
    AddVector --> CleanupNodes[Clear PC Nodes]
    
    CleanupNodes --> CheckCustom{Has Custom Path?}
    CheckCustom -->|Yes| RestoreProps[Restore Original Properties]
    CheckCustom -->|No| Unlock2[Unlock dcaMutex]
    RestoreProps --> Unlock2
    
    Unlock2 --> Return2[Return rc]
```

