# Smart Wardrobe - End-to-End API Smoke Test
$ErrorActionPreference = "Stop"

$port = 18080
$tempDataFile = "backend/data/test_api_temp_wardrobe.csv"
$baseUrl = "http://127.0.0.1:$port"

# Clean up any leftover temporary files
if (Test-Path $tempDataFile) { Remove-Item $tempDataFile -Force }
if (Test-Path "$tempDataFile.tmp") { Remove-Item "$tempDataFile.tmp" -Force }

Write-Host "========================================"
Write-Host "Smart Wardrobe - API Smoke Test Suite"
Write-Host "========================================"

# Check if port is already occupied
$portOccupied = Get-NetTCPConnection -LocalPort $port -State Listen -ErrorAction SilentlyContinue
if ($portOccupied) {
    Write-Error "Port $port is already occupied! Terminate existing process before running tests."
    exit 1
}

# 1. Start server with isolated temporary data path
Write-Host "[1/20] Starting server on port $port with temporary data file $tempDataFile..."
$proc = Start-Process -FilePath "backend/smart_wardrobe_server.exe" -ArgumentList "$port", "$tempDataFile" -PassThru -NoNewWindow

try {
    # 2. Wait for health check endpoint with bounded timeout
    Write-Host "[2/20] Polling GET /api/health..."
    $healthy = $false
    $timeoutSeconds = 10
    $startTime = Get-Date

    while (-not $healthy -and ((Get-Date) - $startTime).TotalSeconds -lt $timeoutSeconds) {
        Start-Sleep -Milliseconds 300
        try {
            $healthRes = Invoke-RestMethod -Uri "$baseUrl/api/health" -Method Get -TimeoutSec 2
            if ($healthRes.status -eq "ok") {
                $healthy = $true
            }
        } catch {
            # Still starting up
        }
    }

    if (-not $healthy) {
        throw "Server failed to respond to /api/health within $timeoutSeconds seconds."
    }
    Write-Host "       Health check OK: status=$($healthRes.status), items=$($healthRes.itemCount)"

    # 3. Confirm initial clothing list loads (seeded from demo wardrobe)
    Write-Host "[3/20] GET /api/clothes..."
    $clothesRes = Invoke-RestMethod -Uri "$baseUrl/api/clothes" -Method Get
    $items = if ($clothesRes -is [array]) { $clothesRes } elseif ($clothesRes.PSObject.Properties['clothes']) { $clothesRes.clothes } else { @($clothesRes) }
    if ($items.Count -lt 1) {
        throw "Initial clothes list should contain seeded items, got $($items.Count)"
    }
    $initialCount = $items.Count
    Write-Host "       Initial item count: $initialCount"

    # 4. Add a valid clothing item (POST /api/clothes)
    Write-Host "[4/20] POST /api/clothes (Add new item)..."
    $newItemPayload = @{
        type = "Top"
        subtype = "Hoodie"
        color = "Navy"
        occasions = @("Casual", "Active")
        seasons = @("Winter")
    } | ConvertTo-Json

    $addRes = Invoke-RestMethod -Uri "$baseUrl/api/clothes" -Method Post -Body $newItemPayload -ContentType "application/json"
    $newId = $addRes.id
    if (-not $newId -or $addRes.subtype -ne "Hoodie" -or $addRes.color -ne "Navy") {
        throw "Failed to add valid clothing item. Response: $($addRes | ConvertTo-Json)"
    }
    Write-Host "       Added item successfully with assigned ID: $newId"

    # 5. Verify created item appears in list
    Write-Host "[5/20] Verify item in GET /api/clothes..."
    $afterAdd = Invoke-RestMethod -Uri "$baseUrl/api/clothes" -Method Get
    $afterItems = if ($afterAdd -is [array]) { $afterAdd } elseif ($afterAdd.PSObject.Properties['clothes']) { $afterAdd.clothes } else { @($afterAdd) }
    $found = $afterItems | Where-Object { [int]$_.id -eq [int]$newId }
    if (-not $found -or $afterItems.Count -ne ($initialCount + 1)) {
        throw "Created item ID $newId not found in updated clothing list."
    }
    Write-Host "       Verified item ID $newId is present in wardrobe."
    Write-Host "       Verified item ID $newId is present in wardrobe."

    # 6. Reject malformed JSON
    Write-Host "[6/20] Testing rejection of malformed JSON..."
    $malformedRejected = $false
    try {
        Invoke-WebRequest -Uri "$baseUrl/api/clothes" -Method Post -Body "{ this is invalid json }" -ContentType "application/json"
    } catch {
        if ($_.Exception.Response.StatusCode.value__ -eq 400) {
            $malformedRejected = $true
        }
    }
    if (-not $malformedRejected) {
        throw "Server should return 400 Bad Request for malformed JSON."
    }
    Write-Host "       Malformed JSON correctly rejected with 400."

    # 7. Reject invalid subtype/category combination
    Write-Host "[7/20] Testing rejection of category mismatch..."
    $mismatchRejected = $false
    $mismatchPayload = @{
        type = "Shoes"
        subtype = "T-Shirt" # T-Shirt is a Top, not Shoes!
        color = "Red"
        occasions = @("Casual")
        seasons = @("Summer")
    } | ConvertTo-Json

    try {
        Invoke-WebRequest -Uri "$baseUrl/api/clothes" -Method Post -Body $mismatchPayload -ContentType "application/json"
    } catch {
        if ($_.Exception.Response.StatusCode.value__ -eq 400) {
            $mismatchRejected = $true
        }
    }
    if (-not $mismatchRejected) {
        throw "Category/subtype mismatch should be rejected with 400."
    }
    Write-Host "       Category mismatch correctly rejected with 400."

    # 8. Reject unsupported subtype
    Write-Host "[8/20] Testing rejection of unsupported subtype..."
    $badSubtypeRejected = $false
    $badSubtypePayload = @{
        type = "Top"
        subtype = "Sombrero"
        color = "Yellow"
        occasions = @("Casual")
        seasons = @("Summer")
    } | ConvertTo-Json

    try {
        Invoke-WebRequest -Uri "$baseUrl/api/clothes" -Method Post -Body $badSubtypePayload -ContentType "application/json"
    } catch {
        if ($_.Exception.Response.StatusCode.value__ -eq 400) {
            $badSubtypeRejected = $true
        }
    }
    if (-not $badSubtypeRejected) {
        throw "Unsupported subtype should be rejected with 400."
    }
    Write-Host "       Unsupported subtype correctly rejected with 400."

    # 9. Update an existing item (PUT /api/clothes/{id})
    Write-Host "[9/20] PUT /api/clothes/$newId (Update item)..."
    $updatePayload = @{
        type = "Top"
        subtype = "Hoodie"
        color = "Black" # Changed from Navy to Black
        occasions = @("Casual", "Active", "Party")
        seasons = @("Winter", "All Season")
    } | ConvertTo-Json

    $updateRes = Invoke-RestMethod -Uri "$baseUrl/api/clothes/$newId" -Method Put -Body $updatePayload -ContentType "application/json"
    if ($updateRes.color -ne "Black") {
        throw "Item update failed to change color to Black."
    }
    Write-Host "       Updated item ID $newId successfully."

    # 10. Verify updated attributes returned
    Write-Host "[10/20] Verify updated attributes via GET /api/clothes..."
    $afterUpdate = Invoke-RestMethod -Uri "$baseUrl/api/clothes" -Method Get
    $updatedItems = if ($afterUpdate -is [array]) { $afterUpdate } elseif ($afterUpdate.PSObject.Properties['clothes']) { $afterUpdate.clothes } else { @($afterUpdate) }
    $updatedFound = $updatedItems | Where-Object { [int]$_.id -eq [int]$newId }
    if ($updatedFound.color -ne "Black" -or $updatedFound.occasions.Count -ne 3) {
        throw "Attributes not correctly reflected after update."
    }
    Write-Host "       Verified updated attributes: color=$($updatedFound.color), occasions=$($updatedFound.occasions -join '|')"

    # 11. Delete an existing item (DELETE /api/clothes/{id})
    Write-Host "[11/20] DELETE /api/clothes/$newId..."
    $deleteRes = Invoke-RestMethod -Uri "$baseUrl/api/clothes/$newId" -Method Delete
    if (-not $deleteRes.success) {
        throw "Delete operation reported failure."
    }
    Write-Host "       Deleted item ID $newId successfully."

    # 12. Handle deletion of nonexistent item
    Write-Host "[12/20] Testing deletion of nonexistent item (ID 99999)..."
    $nonexistentDelete404 = $false
    try {
        Invoke-WebRequest -Uri "$baseUrl/api/clothes/99999" -Method Delete
    } catch {
        if ($_.Exception.Response.StatusCode.value__ -eq 404) {
            $nonexistentDelete404 = $true
        }
    }
    if (-not $nonexistentDelete404) {
        throw "Deleting nonexistent item should return 404."
    }
    Write-Host "       Nonexistent deletion correctly returned 404."

    # 13. Generate outfits using occasion and season
    Write-Host "[13/20] POST /api/outfits/generate (Casual, All Season)..."
    $recPayload = @{
        occasion = "Casual"
        season = "All Season"
    } | ConvertTo-Json

    $recRes = Invoke-RestMethod -Uri "$baseUrl/api/outfits/generate" -Method Post -Body $recPayload -ContentType "application/json"
    if (-not $recRes.outfits -or $recRes.outfits.Count -eq 0) {
        throw "Outfit generation returned no outfits for Casual All Season."
    }
    Write-Host "       Generated $($recRes.outfits.Count) recommended outfits."

    # 14. Validate response structure and field types
    Write-Host "[14/20] Validating outfit response structure..."
    $firstOutfit = $recRes.outfits[0]
    if (-not $firstOutfit.top -or -not $firstOutfit.bottom -or -not $firstOutfit.shoes -or -not $firstOutfit.score) {
        throw "Outfit missing required fields: top, bottom, shoes, score."
    }
    Write-Host "       Top Outfit: $($firstOutfit.top) + $($firstOutfit.bottom) + $($firstOutfit.shoes) (Score: $($firstOutfit.score))"

    # 15. Verify score bounds and rationale strings
    Write-Host "[15/20] Verifying score bounds [0, 100] and rationale..."
    foreach ($o in $recRes.outfits) {
        if ($o.score -lt 0 -or $o.score -gt 100) {
            throw "Score $($o.score) is outside valid range [0, 100]"
        }
        if (-not $o.reason -or $o.reason.Count -eq 0) {
            throw "Outfit missing explanatory rationale array"
        }
    }
    Write-Host "       All outfits have bounded scores and non-empty reasons."

    # 16. Verify server returns valid empty result when no items match
    Write-Host "[16/20] Testing empty recommendation for impossible criteria..."
    $emptyPayload = @{
        occasion = "NonExistentOccasion12345"
        season = "Winter"
    } | ConvertTo-Json
    $emptyRecRes = Invoke-RestMethod -Uri "$baseUrl/api/outfits/generate" -Method Post -Body $emptyPayload -ContentType "application/json"
    if ($null -eq $emptyRecRes.outfits -or $emptyRecRes.outfits.Count -ne 0) {
        throw "Expected empty outfits array for non-matching criteria."
    }
    Write-Host "       Valid empty array returned for non-matching criteria."

    # 17. Test CORS preflight (OPTIONS)
    Write-Host "[17/20] Testing CORS preflight OPTIONS request..."
    $corsRes = Invoke-WebRequest -Uri "$baseUrl/api/clothes" -Method Options -UseBasicParsing
    $allowOrigin = $corsRes.Headers["Access-Control-Allow-Origin"]
    if ($corsRes.StatusCode -ne 200 -or -not $allowOrigin) {
        throw "CORS OPTIONS preflight failed or missing Access-Control-Allow-Origin header."
    }
    Write-Host "       CORS preflight OK: Access-Control-Allow-Origin: $allowOrigin"

    # 18. Stop server reliably
    Write-Host "[18/20] Stopping server process..."
    Stop-Process -Id $proc.Id -Force
    $proc.WaitForExit(3000)
    Write-Host "       Server process stopped."

    # 19. Restart server with same temporary data path and verify persistence
    Write-Host "[19/20] Restarting server to verify data persistence..."
    $proc2 = Start-Process -FilePath "backend/smart_wardrobe_server.exe" -ArgumentList "$port", "$tempDataFile" -PassThru -NoNewWindow
    try {
        # Wait for startup
        $restarted = $false
        $startTime = Get-Date
        while (-not $restarted -and ((Get-Date) - $startTime).TotalSeconds -lt 10) {
            Start-Sleep -Milliseconds 300
            try {
                $h = Invoke-RestMethod -Uri "$baseUrl/api/health" -Method Get -TimeoutSec 2
                if ($h.status -eq "ok") { $restarted = $true }
            } catch {}
        }
        if (-not $restarted) { throw "Restarted server failed to respond." }

        # Verify item count matches initialCount (since we added and deleted item newId)
        $persistCheck = Invoke-RestMethod -Uri "$baseUrl/api/clothes" -Method Get
        $persistedItems = if ($persistCheck -is [array]) { $persistCheck } elseif ($persistCheck.PSObject.Properties['clothes']) { $persistCheck.clothes } else { @($persistCheck) }
        if ($persistedItems.Count -ne $initialCount) {
            throw "Persistence check failed: expected $initialCount items, found $($persistedItems.Count)"
        }
        Write-Host "       Persistence verified: exactly $initialCount items restored on restart."
    } finally {
        Stop-Process -Id $proc2.Id -Force
        $proc2.WaitForExit(3000)
    }

    # 20. Clean up temporary files
    Write-Host "[20/20] Cleaning up temporary test files..."
    if (Test-Path $tempDataFile) { Remove-Item $tempDataFile -Force }
    if (Test-Path "$tempDataFile.tmp") { Remove-Item "$tempDataFile.tmp" -Force }
    Write-Host "       Cleanup complete."

    Write-Host "========================================"
    Write-Host "ALL 20 API SMOKE TESTS PASSED!"
    Write-Host "========================================"
} catch {
    Write-Error "Smoke test failed: $_"
    if ($proc -and -not $proc.HasExited) {
        Stop-Process -Id $proc.Id -Force
    }
    if (Test-Path $tempDataFile) { Remove-Item $tempDataFile -Force }
    if (Test-Path "$tempDataFile.tmp") { Remove-Item "$tempDataFile.tmp" -Force }
    exit 1
}
