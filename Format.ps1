function Format-FilesInDirectory {
    param (
        [string]$Path
    )

    $extensions = "*.cpp", "*.c", "*.cc", "*.cxx", "*.h", "*.hpp", "*.hxx", "*.inl",
                  "*.glsl", "*.vert", "*.frag", "*.comp", "*.geom", "*.tesc", "*.tese",
                  "*.proto", "*.cs"

    Get-ChildItem -Path $Path -Recurse -Include $extensions |
        Where-Object { $_.Name -notmatch "generated" } | 
        ForEach-Object {
            $original = Get-Content $_.FullName -Raw
            clang-format -i $_.FullName
            $formatted = Get-Content $_.FullName -Raw

            if ($original -ne $formatted) {
                Write-Host "$($_.FullName)" -ForegroundColor Green
            }
            else {
                Write-Host "$($_.FullName)" -ForegroundColor Gray
            }
        }
}


Write-Host "Starting Formatting..." -ForegroundColor Cyan

Format-FilesInDirectory -Path ".\src"
Format-FilesInDirectory -Path ".\examples"
Format-FilesInDirectory -Path ".\stdlib"

Write-Host "Finished Formatting!" -ForegroundColor Cyan
