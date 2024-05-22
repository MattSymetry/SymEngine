param(
    [string]$inputFile
)

if (-not $inputFile) {
    Write-Host "Usage: .\resize_images.ps1 -inputFile <input_file.png>"
    exit
}

# Define the sizes you want to generate
$sizes = @("256x256")

# Get the base name without the extension
$basename = [System.IO.Path]::GetFileNameWithoutExtension($inputFile)

# Loop through each size and generate the corresponding PNG
$pngFiles = @()
foreach ($size in $sizes) {
    $dimensions = $size.Split('x')
    $width = $dimensions[0]
    $height = $dimensions[1]
    $outputFile = "${basename}_${size}.png"
    & 'C:\Program Files\Inkscape\bin\inkscape.com' $inputFile --export-type=png --export-width=$width --export-height=$height --export-filename=$outputFile
    Write-Host "Generated $outputFile"
    $pngFiles += $outputFile
}

# Create an .ico file from the generated PNGs using ImageMagick
$icoOutputFile = "${basename}.ico"
& magick convert $pngFiles -colors 256 $icoOutputFile
Write-Host "Generated $icoOutputFile"

# Move the .ico file to the ../resources/ directory
$destinationPath = "../resources/"
if (-not (Test-Path -Path $destinationPath)) {
    New-Item -ItemType Directory -Path $destinationPath
}
Move-Item -Path $icoOutputFile -Destination $destinationPath
Write-Host "Moved $icoOutputFile to $destinationPath"

# Delete the generated PNG files
foreach ($pngFile in $pngFiles) {
    Remove-Item $pngFile
    Write-Host "Deleted $pngFile"
}
