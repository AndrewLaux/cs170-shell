main: 
	g++ simple_shell.cpp task.h task_manager.h task_manager.cpp -o simple_shell

clean:
	rm -rf *.o *.exe