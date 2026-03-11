set logging enabled on
set logging file dynamic_gdb_callstack.log
set pagination off
set confirm off

# Simple approach - let the shell timeout handle stopping
set breakpoint pending on

printf "=== Starting MEMORY LEAK ANALYSIS (configurable timeout) ===\n"

# Track SSL ALLOCATIONS - simple continue commands
break EVP_CIPHER_fetch
commands 1
printf "EVP_CIPHER_fetch hit\n"
continue
end

break CRYPTO_zalloc
commands 2
printf "CRYPTO_zalloc hit\n"
continue
end

break CRYPTO_malloc  
commands 3
printf "CRYPTO_malloc hit\n"
continue
end

break RAND_get0_primary
commands 4
printf "RAND_get0_primary hit\n"
continue
end

# Track SSL DEALLOCATIONS
break CRYPTO_free
commands 5
printf "CRYPTO_free hit\n"
continue
end

break EVP_CIPHER_free
commands 6
printf "EVP_CIPHER_free hit\n"
continue
end

break SSL_CTX_free
commands 7
printf "SSL_CTX_free hit\n"
continue
end

printf "=== Breakpoints set - will be stopped by timeout ===\n"

# Run the program - timeout will stop it automatically  
run

printf "=== Analysis completed ===\n"
quit
