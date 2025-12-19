from setuptools import find_packages, setup

package_name = 'social_bt_nodes'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', ['launch/tracker.launch.py']),
        ('share/' + package_name + '/launch', ['launch/follow_behavior.launch.py']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='roi',
    maintainer_email='rodrigo.perez@urjc.es',
    description='Reusable BehaviorTree nodes for social robotics (motion, perception, HRI)',
    license='Apache 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'yolo_to_standard = social_bt_nodes.yolo_to_standard:main',
            'entity_tracker_fake_3d = social_bt_nodes.entity_tracker_fake_3d:main',
        ],
    },
)
