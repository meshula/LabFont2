// WebGPU Test Runner
document.addEventListener('DOMContentLoaded', function() {
    const output = document.getElementById('output');
    const canvas = document.getElementById('canvas');
    
    // Log function for test output
    function log(message, isError = false) {
        const div = document.createElement('div');
        div.textContent = message;
        if (isError) {
            div.style.color = 'red';
        }
        output.appendChild(div);
    }
    
    // Check if WebGPU is supported
    if (!navigator.gpu) {
        log('WebGPU is not supported in this browser.', true);
        return;
    }
    
    log('WebGPU is supported! Running tests...');
    
    // The C++ code will call this function when tests are complete
    window.reportTestResults = function(passed, total, errorMessage) {
        if (passed === total) {
            log(`All tests passed! (${passed}/${total})`);
        } else {
            log(`${passed}/${total} tests passed.`, true);
            if (errorMessage) {
                log(`Error: ${errorMessage}`, true);
            }
        }
    };
    
    // The C++ code will call this function to log test progress
    window.logTestMessage = function(message, isError) {
        log(message, isError);
    };
    
    // The main test function will be called by the Emscripten module
    // when it's ready
    window.runTests = function() {
        log('Starting WebGPU tests...');
        
        // The actual test execution is handled by the C++ code
        // This function just sets up the environment
        
        // Return the canvas context for the tests to use
        return {
            canvas: canvas,
            width: canvas.width,
            height: canvas.height
        };
    };
});
